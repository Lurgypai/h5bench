#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <chrono>

#include <cstring>

#include <hdf5.h>
#include <mpi.h>

#include "../encryption_wrapper/EncryptionLibrary.h"
#include "../encryption_wrapper/ELgcrypt.h"

class Timer {
public:
    void reset() {
        start = std::chrono::high_resolution_clock::now();
    }
    double getElapsed() {
        auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::high_resolution_clock::now() - start);
        return elapsed.count();
    }
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
};

// size of elements in the opaque type, the count in the configuration is in terms of these blocks
constexpr std::size_t SHARED_BLOCK_SIZE = 16;

struct Dataset {
    size_t count;
    std::string algorithm;
    std::string library;
};

static inline bool isValidDataset(const Dataset& dataset) {
    return dataset.count > 0 &&
        (dataset.algorithm == "aes256" || dataset.algorithm == "chacha20" || dataset.algorithm == "none") &&
        (dataset.library == "nettle" || dataset.library == "gcrypt" || dataset.library == "none");
}

int main(int argc, char** argv) {
    MPI_Init(NULL, NULL);
    int processCount;
    MPI_Comm_size(MPI_COMM_WORLD, &processCount);
    int myRank;
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);

    if(argc != 2) {
        std::cout << "Incorrect usage.\n";
        std::cout << "Usage: " << argv[0] << " <config>\n";
        return 1;
    }

    std::string configFileName{argv[1]};
    std::ifstream configFile{configFileName};

    if(!configFile.good()) {
        std::cerr << "ERROR: Unable to open file \"" << configFileName << "\"\n";
        return 1;
    }

    /* =========================== PARSE CONFIG ========================== */
    std::vector<Dataset> datasetTemplates;
    Dataset* curDataset = nullptr;
    std::string line;
    while(std::getline(configFile, line)) {
        // check for beginning of dataset
        if(line == "dataset") {
            // check if current dataset is valid (specified all values)
            if(curDataset != nullptr && !isValidDataset(*curDataset)) {
                // error out because we didn't read a full dataset
                std::cerr << "ERROR: Invalid dataset description in config file\n";
                return 1;
            }

            // generate new dataset
            datasetTemplates.emplace_back(Dataset{});
            curDataset = &datasetTemplates.back();
        }

        // fill in dataset values
        else {
            if(curDataset == nullptr) {
                std::cerr << "ERROR: Initial dataset hasn't been started (did you forget \"dataset\" at the beginning of the file?\n";
                return 1;
            }
            // split line at = sign
            auto splitPos = line.find('=');
            if(splitPos == std::string::npos || splitPos == line.size() - 1) {
                std::cerr << "ERROR: Unable to parse line \"" << line << "\"\n";
                return 1;
            }

            std::string front = line.substr(0, splitPos);
            std::string back = line.substr(splitPos + 1);
            if(front == "count") {
                try {
                    curDataset->count = std::stoull(back);
                } catch (std::invalid_argument e) {
                    std::cerr << "ERROR: Unable to parse count, value \"" << back << "\"\n";
                    return 1;
                }

                if(curDataset->count % processCount != 0) {
                    std::cerr << "ERROR: count \"" << curDataset->count << "\" must be a multiple of the process count, \"" << processCount << "\"k\n";
                }
            } else if(front == "library") {
                curDataset->library = back;
            } else if (front == "algorithm") {
                curDataset->algorithm = back;
            } else {
                std::cerr << "ERROR: Unable to parse line \"" << line << "\"\n";
                return 1;
            }
        }
    }
    /* =========================== END PARSE CONFIG ========================== */

    MPI_Barrier(MPI_COMM_WORLD);

    /* =========================== PREP FILE ========================== */
    hid_t fapl = H5Pcreate(H5P_FILE_ACCESS);
    H5Pset_fapl_mpio(fapl, MPI_COMM_WORLD, MPI_INFO_NULL);
    // TODO: Add alignment variables
    // TODO: Fix file name
    hid_t fileId = H5Fcreate("output.hdf5", H5F_ACC_TRUNC, H5P_DEFAULT, fapl);
    H5Pclose(fapl);
    /* =========================== END PREP FILE ========================== */

    MPI_Barrier(MPI_COMM_WORLD);

    /* =========================== PREP DATASETS ========================== */
    hid_t aesOpaque = H5Tcreate(H5T_OPAQUE, SHARED_BLOCK_SIZE);

    std::vector<hid_t> datasetIds;
    datasetIds.resize(datasetTemplates.size());
    for(int i = 0; i != datasetTemplates.size(); ++i) {
        const auto& datasetTemplate = datasetTemplates[i];
        auto& dsetId = datasetIds[i];
        std::string datasetName{"dataset"};
        datasetName += std::to_string(i);
        hsize_t spaceSize[1] = {datasetTemplate.count};
        hid_t fSpace = H5Screate_simple(1, spaceSize, NULL);
        dsetId = H5Dcreate2(fileId, datasetName.c_str(), aesOpaque, fSpace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    }
    /* =========================== END PREP DATASETS ========================== */

    MPI_Barrier(MPI_COMM_WORLD);

    /* =========================== PERFORM IO ========================== */
    
    Timer totalIOTimer;
    totalIOTimer.reset();
    Timer encryptionTimer;
    double encryptionTime = 0.0;
    Timer writeTimer;
    double writeTime = 0.0;
    

    for(int i = 0; i != datasetTemplates.size(); ++i) {
        const auto& datasetTemplate = datasetTemplates[i];
        const auto& dsetId = datasetIds[i];

        const std::size_t ioCount = datasetTemplate.count / processCount;
        const std::size_t ioSize = ioCount * SHARED_BLOCK_SIZE;

        std::vector<char> ciphertextBuffer;
        ciphertextBuffer.resize(ioSize);

        /* --------------- ENCRYPTION --------------- */
        encryptionTimer.reset();
        if(datasetTemplate.library != "none") {
            // generate encryption context
            std::unique_ptr<EncryptionLibrary> el;
            if(datasetTemplate.library == "gcrypt") {
                el = std::make_unique<ELgcrypt>();
            }
            else if(datasetTemplate.library == "nettle") {
                el = std::make_unique<ELgcrypt>();
            }

            if(datasetTemplate.algorithm == "aes256") {
                el->prepare(Algorithm::aes256);
            }
            else if(datasetTemplate.algorithm == "chacha20") {
                el->prepare(Algorithm::chacha20);
            }

            std::string key = el->makeKey();
            el->setKey(key.data(), key.size());
            std::string nonce = el->makeNonce();
            el->setNonce(nonce.data(), nonce.size());

            // allocate a buffers
            std::vector<char> plaintextBuffer;
            plaintextBuffer.resize(ioSize);

            // apply encryption
            el->encrypt(plaintextBuffer.data(), plaintextBuffer.size(), ciphertextBuffer.data(), ciphertextBuffer.size());
        } 
        encryptionTime += encryptionTimer.getElapsed();


        /* --------------- IO --------------- */
        writeTimer.reset();
        // do write
        hsize_t spaceSize[1] = {datasetTemplate.count};
        hid_t fSpace = H5Screate_simple(1, spaceSize, NULL);
        hsize_t memSpaceSize[1] = {ioCount};
        hid_t mSpace = H5Screate_simple(1, memSpaceSize, NULL);

        hsize_t offset[1] = {ioCount * myRank};
        hsize_t blkCount[1] = {1};
        H5Sselect_hyperslab(fSpace, H5S_SELECT_SET, offset, NULL, blkCount, &ioCount);
        hid_t dxpl = H5Pcreate(H5P_DATASET_XFER);
        H5Pset_dxpl_mpio(dxpl, H5FD_MPIO_COLLECTIVE);
        H5Dwrite(dsetId, aesOpaque, mSpace, fSpace, dxpl, ciphertextBuffer.data());
        writeTime += writeTimer.getElapsed();

        H5Pclose(dxpl);
        H5Sclose(fSpace);
        H5Sclose(mSpace);
    }
    /* =========================== END PERFORM IO ========================== */

    double ioTime = totalIOTimer.getElapsed();

    // logging performed only by rank 0
    if(myRank != 0) return 0;

    double ioTimeS = ioTime / (1000.0 * 1000.0 * 1000.0);
    double writeTimeS = writeTime / (1000.0 * 1000.0 * 1000.0);
    double encryptionTimeS = encryptionTime / (1000.0 * 1000.0 * 1000.0);

    std::cout << "Total time: " << ioTimeS << '\n';
    std::cout << "Write time: " << writeTimeS << '\n';
    std::cout << "Encryption time: " << encryptionTimeS << '\n';
    
    std::string outName = configFileName + std::string{"-out.csv"};
    std::ofstream outFile{outName};

    if(!outFile.good()) {
        std::cerr << "Error, unable to open output file \"out.csv\" for writing.\n";
        return 1;
    }

    outFile << "name, value\n";
    outFile << "total, " << ioTimeS << '\n';
    outFile << "write, " << writeTimeS << '\n';
    outFile << "encryption, " << encryptionTimeS << '\n';

    return 0;
}


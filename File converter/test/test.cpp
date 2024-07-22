#include "../file_converter.h"
#include <iostream>
#include <cassert>

void printBits(const std::vector<uint8_t>& bits) {
    for (const auto& bit : bits) {
        std::cout << static_cast<int>(bit);
    }
    std::cout << std::endl;
}

int main() {
    const std::string originalFilePath = "../test.txt"; // Path to the original file
    const std::string outputDir = "."; // Directory to store the output file
    std::string fileName; // Will hold the file name

    try {
        // Convert file to bits with metadata
        std::vector<uint8_t> fileBits = fileToBitsWithMetadata(originalFilePath, fileName);

        // Output number of bits and the bits themselves
        std::cout << "Number of bits: " << fileBits.size() << std::endl;
        // std::cout << "Bits: ";
        // printBits(fileBits);

        // Convert bits back to file with metadata
        bitsToFileWithMetadata(outputDir, fileBits, fileName);

        // Read the output file back to bits to compare
        std::vector<uint8_t> outputBits = fileToBitsWithMetadata(fileName, fileName);

        // Output number of bits for the restored file
        std::cout << "Number of bits (restored file): " << outputBits.size() << std::endl;
        // std::cout << "Bits (restored file): ";
        // printBits(outputBits);

        // Verify the file data matches the original data
        assert(fileBits == outputBits);

        std::cout << "Test passed: The original file and the output file are identical." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
    }

    return 0;
}

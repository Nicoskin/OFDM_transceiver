#ifndef FILE_CONVERTER_H
#define FILE_CONVERTER_H

#include <vector>
#include <string>
#include <cstdint>

std::vector<uint8_t> fileToBitsWithMetadata(const std::string& filePath, std::string& fileName);
void bitsToFileWithMetadata(const std::string& outputDir, const std::vector<uint8_t>& bits, std::string& fileName);

#endif // FILE_CONVERTER_H

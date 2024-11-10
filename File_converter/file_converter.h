#ifndef FILE_CONVERTER_H
#define FILE_CONVERTER_H

#include <vector>
#include <string>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <filesystem>
#include <random>

std::vector<uint8_t> file2bits(const std::string& filePath);
void bits2file(const std::string& outputDir, const std::vector<uint8_t>& bits);

std::vector<uint8_t> string2bits(const std::string& str);
void bits2string(const std::vector<uint8_t>& bits);

std::vector<uint8_t> generateRandBits(size_t n, unsigned int seed = 0);

#endif // FILE_CONVERTER_H

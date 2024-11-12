#include "file_converter.h"

#include <iostream>


namespace fs = std::filesystem;

std::vector<uint8_t> file2bits(const std::string& filePath) {
    std::ifstream inputFile(filePath, std::ios::binary);
    if (!inputFile) {
        throw std::runtime_error("Failed to open file for reading: " + filePath);
    }

    // Получить имя файла и расширение
    std::string fileName = fs::path(filePath).filename().string();

    // Считать файл в вектор байтов
    std::vector<uint8_t> fileData((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());

    // Преобразовать байты в биты
    std::vector<uint8_t> bits;

    // Добавить длину имени файла как первые 8 битов
    uint8_t fileNameLength = static_cast<uint8_t>(fileName.size());
    for (int i = 7; i >= 0; --i) {
        bits.push_back((fileNameLength >> i) & 1);
    }

    // Добавить имя файла в виде битов
    for (const auto& ch : fileName) {
        for (int i = 7; i >= 0; --i) {
            bits.push_back((ch >> i) & 1);
        }
    }

    // Добавить данные файла в виде битов
    for (const auto& byte : fileData) {
        for (int i = 7; i >= 0; --i) {
            bits.push_back((byte >> i) & 1);
        }
    }

    return bits;
}

void bits2file(const std::string& outputDir, const std::vector<uint8_t>& bits) {
    if (bits.size() < 8) {
        throw std::runtime_error("Invalid bits data");
    }

    // Extract the length of the file name
    uint8_t fileNameLength = 0;
    for (int i = 0; i < 8; ++i) {
        fileNameLength = (fileNameLength << 1) | bits[i];
    }

    if (bits.size() < 8 + fileNameLength * 8) {
        throw std::runtime_error("Invalid bits data");
    }

    // Extract the file name
    std::string fileName;
    for (int i = 8; i < 8 + fileNameLength * 8; i += 8) {
        uint8_t ch = 0;
        for (int j = 0; j < 8; ++j) {
            ch = (ch << 1) | bits[i + j];
        }
        fileName.push_back(static_cast<char>(ch));
    }

    // Extract the file data
    std::vector<uint8_t> fileData;
    for (size_t i = 8 + fileNameLength * 8; i < bits.size(); i += 8) {
        uint8_t byte = 0;
        for (int j = 0; j < 8; ++j) {
            byte = (byte << 1) | bits[i + j];
        }
        fileData.push_back(byte);
    }

    // Create the output file path
    fs::path outputPath(outputDir);
    if (!fs::exists(outputPath)) {
        fs::create_directories(outputPath);
    }
    std::string outputFilePath = fs::path(outputDir) / fileName;
    std::ofstream outputFile(outputFilePath, std::ios::binary);
    if (!outputFile) {
        throw std::runtime_error("Failed to open file for writing: " + outputFilePath);
    }

    // Write the file data
    outputFile.write(reinterpret_cast<const char*>(fileData.data()), fileData.size());
    std::cout << "File saved" << std::endl;
}

std::vector<uint8_t> generateRandBits(size_t n, unsigned int seed) {
    std::vector<uint8_t> bits(n);
    
    // Если сид равен 0, используем случайный генератор с временем как сидом
    std::mt19937 generator(seed == 0 ? std::random_device{}() : seed);
    std::uniform_int_distribution<uint8_t> distribution(0, 1);
    
    for (size_t i = 0; i < n; ++i) {
        bits[i] = distribution(generator);
    }
    
    return bits;
}

// Функция для преобразования строки в вектор бит
std::vector<uint8_t> string2bits(const std::string& str) {
    std::vector<uint8_t> bits;
    for (char ch : str) {
        for (int i = 7; i >= 0; --i) {  // Извлекаем биты от старшего к младшему
            bits.push_back((ch >> i) & 1);
        }
    }
    return bits;
}

// Функция для преобразования вектора бит обратно в строку и вывод в терминал
void bits2string(const std::vector<uint8_t>& bits) {
    std::string str;
    for (size_t i = 0; i < bits.size(); i += 8) {
        uint8_t byte = 0;
        for (int j = 0; j < 8; ++j) {  // Собираем 8 бит в один байт
            byte = (byte << 1) | bits[i + j];
        }
        str.push_back(static_cast<char>(byte));
    }
    std::cout << "  -> " << str << std::endl;
}
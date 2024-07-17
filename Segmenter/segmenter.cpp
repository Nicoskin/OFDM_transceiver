#include "segmenter.h"
#include <vector>
#include <cstdint>
#include <iostream>
#include <bitset>
#include <random>

// Конструктор класса Segmenter
Segmenter::Segmenter(
    uint32_t maxLenLine,
    uint32_t segmentNumBits,
    //uint32_t totalSegmentBits, 
    uint32_t usefulBits,
    uint32_t crcBits)
    : maxLenLine(maxLenLine), segmentNumBits(segmentNumBits), //totalSegmentBits(totalSegmentBits),
      usefulBits(usefulBits), crcBits(crcBits) {}

// Функция разбиения вектора бит на сегменты
std::vector<std::vector<uint8_t>> Segmenter::segment(const std::vector<uint8_t>& bits) {
    std::vector<std::vector<uint8_t>> segments;
    setlocale(LC_ALL, "Russian");

    uint32_t maxLenLineInSegment = maxLenLine - segmentNumBits /*- totalSegmentBits*/ - usefulBits - crcBits;
    uint32_t totalSegments = (bits.size() + maxLenLineInSegment - 1) / maxLenLineInSegment;

    //DEBUG
    std::cout << "Всего бит: " << bits.size()
    << "\nОбщее количество сегментов: " << totalSegments
    << "\nМаксимальное количество полезных бит в сегменте: " << maxLenLineInSegment << std::endl;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 1);

    for (uint32_t i = 0; i < totalSegments; ++i) {
        std::vector<uint8_t> segment;

        // Номер сегмента
        for (int j = segmentNumBits - 1; j >= 0; --j) {
            segment.push_back((i >> j) & 1);
        }

        // Общее количество сегментов
        // for (int j = totalSegmentBits - 1; j >= 0; --j) {
        //     segment.push_back((totalSegments >> j) & 1);
        // }

        // Длина полезных бит
        uint32_t usefulBitsLength = (i == totalSegments - 1) ? (bits.size() % maxLenLineInSegment) : maxLenLineInSegment;
        if (usefulBitsLength == 0) usefulBitsLength = maxLenLineInSegment; // Handle the case where bits.size() is a multiple of maxLenLineInSegment

        for (int j = usefulBits - 1; j >= 0; --j) {
            segment.push_back((usefulBitsLength >> j) & 1);
        }

        // Полезные данные
        uint32_t start = i * maxLenLineInSegment;
        for (uint32_t j = 0; j < usefulBitsLength; ++j) {
            segment.push_back(bits[start + j]);
        }

        // Вычисление CRC
        uint64_t crc = computeCRC(segment);
        for (int j = crcBits - 1; j >= 0; --j) {
            segment.push_back((crc >> j) & 1);
        }

        // Добавление случайных битов, если не хватает до maxLenLine
        uint32_t remainingBits = maxLenLine - segment.size();
        for (uint32_t j = 0; j < remainingBits; ++j) {
            segment.push_back(dis(gen));
        }

        //DEBUG
        // std::cout << i << " crc " << crc << "  " << std::bitset<16>(crc) << std::endl;

        segments.push_back(segment);
    }

    return segments;
}

// Функция вычисления CRC
uint64_t Segmenter::computeCRC(const std::vector<uint8_t>& data) {
    const uint64_t polynom = 0xA745; // 1010 0111 0100 0101
    uint64_t crc = (1ULL << crcBits) - 1; // Инициализация CRC с учетом заданного количества бит

    for (auto bit : data) {
        bool xor_flag = crc & (1ULL << (crcBits - 1));
        crc = ((crc << 1) | bit) & ((1ULL << crcBits) - 1); // Обрезаем CRC до нужного количества бит
        if (xor_flag) {
            crc ^= polynom;
        }
    }

    return crc & ((1ULL << crcBits) - 1); // Возвращаем CRC, обрезанный до нужного количества бит
}

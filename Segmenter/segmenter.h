#ifndef SEGMENTER_H
#define SEGMENTER_H

#include <vector>
#include <cstdint>
#include <bitset>
#include <random>

#include "../config.h"

class Segmenter {
public:
    Segmenter(
            uint32_t maxLenLine = MAX_LEN_LINE,
            uint32_t segmentNumBits = SEGMENT_NUM_BITS,
            uint32_t usefulBits = USEFUL_BITS,
            uint32_t crcBits = CRC_BITS
            );

    std::vector<std::vector<uint8_t>> segment(const std::vector<uint8_t>& bits);
    std::vector<int> checkCRC(const std::vector<std::vector<uint8_t>>& segments);
    std::vector<std::vector<uint8_t>> scramble(const std::vector<std::vector<uint8_t>>& data);

private:
    uint32_t maxLenLine;
    uint32_t segmentNumBits;
    uint32_t usefulBits;
    uint32_t crcBits;

    uint64_t computeCRC(const std::vector<uint8_t>& data);
};

#endif // SEGMENTER_H

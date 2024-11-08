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
    void get_size_data_in_slot();
    std::vector<std::vector<uint8_t>> reshape(const std::vector<uint8_t>& bits);
    std::vector<uint8_t> extract_data(const std::vector<std::vector<uint8_t>>& bits);

private:
    uint32_t maxLenLine;
    uint32_t segmentNumBits;
    uint32_t usefulBits;
    uint32_t crcBits;
    uint32_t dataBitsInput;

    uint64_t computeCRC(const std::vector<uint8_t>& data);
};

#endif // SEGMENTER_H

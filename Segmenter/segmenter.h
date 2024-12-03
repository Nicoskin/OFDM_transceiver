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
            uint16_t maxLenLine     = MAX_LEN_LINE,
            uint8_t  segmentNumBits = SEGMENT_NUM_BITS,
            uint8_t  usefulBits     = USEFUL_BITS,
            uint8_t  crcBits        = CRC_BITS,
            uint8_t  flagBits       = 2
            );

    std::vector<std::vector<uint8_t>> segment (const std::vector<uint8_t>& bits, uint8_t flag = 0);
    
    std::vector<std::vector<uint8_t>> scramble(const std::vector<std::vector<uint8_t>>& data);
    std::vector<std::vector<uint8_t>> reshape (const std::vector<uint8_t>& bits);

    std::vector<uint8_t> extract_data(const std::vector<std::vector<uint8_t>>& bits);
                uint8_t  extract_flag(const std::vector<std::vector<uint8_t>>& bits);

    void get_size_data_in_slot() const;

private:
    uint16_t maxLenLine;
    uint8_t  segmentNumBits;
    uint8_t  usefulBits;
    uint8_t  crcBits;
    uint8_t  flagBits;

    uint32_t dataBitsInput;
    uint16_t maxLenLineInSegment;
    uint16_t totalSegments;

    std::vector<int> checkCRC(const std::vector<std::vector<uint8_t>>& segments);
    uint64_t computeCRC(const std::vector<uint8_t>& data);
};

#endif // SEGMENTER_H

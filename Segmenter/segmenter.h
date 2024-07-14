#ifndef SEGMENTER_H
#define SEGMENTER_H

#include <vector>
#include <cstdint>

class Segmenter {
public:
    Segmenter(
            uint32_t maxLenLine = 128,
            uint32_t segmentNumBits = 16,
            //uint32_t totalSegmentBits = 16, 
            uint32_t usefulBits = 32,
            uint32_t crcBits = 16
            );

    std::vector<std::vector<uint8_t>> segment(const std::vector<uint8_t>& bits);

private:
    uint32_t maxLenLine;
    uint32_t segmentNumBits;
    //uint32_t totalSegmentBits;
    uint32_t usefulBits;
    uint32_t crcBits;

    uint64_t computeCRC(const std::vector<uint8_t>& data);
};

#endif // SEGMENTER_H

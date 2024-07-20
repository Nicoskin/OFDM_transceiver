#ifndef SEGMENTER_H
#define SEGMENTER_H

#include <vector>
#include <cstdint>

class Segmenter {
public:
    Segmenter(
            uint32_t maxLenLine = (128-55-6-1)*5*2, // (N_fft - GB - Pilots - 1) * N_OFDM_symbols * QAM_mod
            uint32_t segmentNumBits = 10,
            //uint32_t totalSegmentBits = 16, 
            uint32_t usefulBits = 32,
            uint32_t crcBits = 16
            );

    std::vector<std::vector<uint8_t>> segment(const std::vector<uint8_t>& bits);
    std::vector<std::vector<uint8_t>> scramble(const std::vector<std::vector<uint8_t>>& data);

private:
    uint32_t maxLenLine;
    uint32_t segmentNumBits;
    //uint32_t totalSegmentBits;
    uint32_t usefulBits;
    uint32_t crcBits;

    uint64_t computeCRC(const std::vector<uint8_t>& data);
};

#endif // SEGMENTER_H

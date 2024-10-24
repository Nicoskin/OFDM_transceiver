#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include "../../QAM/qam_mod.h"
#include "../../QAM/qam_demod.h"
#include "../../Segmenter/segmenter.h"
#include "../../OFDM/ofdm_mod.h"
#include "../../OFDM/ofdm_demod.h"
#include "../../File_converter/file_converter.h"

// g++ test2.cpp ../../File_converter/file_converter.cpp ../../QAM/qam_mod.cpp ../../QAM/qam_demod.cpp ../../Segmenter/segmenter.cpp ../../OFDM/ofdm_mod.cpp ../../OFDM/ofdm_demod.cpp ../../OFDM/fft/fft.cpp -o test && ./test

namespace {
    using cd = std::complex<double>;
    using namespace std::complex_literals;  
}


int main() {
    // Сегментер
    Segmenter segmenter;
    auto bits = generateRandBits(500);
    auto segments = segmenter.segment(bits);
    segments = segmenter.scramble(segments);

    QAM_mod qam_mod;
    auto qpsk_mod = qam_mod.modulate(segments, QPSK);

    OFDM_mod ofdm_mod;
    auto ofdm_data = ofdm_mod.modulate(qpsk_mod);

    auto pss = ofdm_mod.mapPSS(); // PSS во времени для поиска


    // OFDM_demod ofdm_demod;
    // auto corr = ofdm_demod.convolve(noisy_signal, pss);


    return 0;
}

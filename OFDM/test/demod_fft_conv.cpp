#include "../ofdm_demod.h"
#include "../fft/fft.h"
#include <iostream>

// g++ --std c++20 demod_fft_conv.cpp ../ofdm_demod.cpp ../fft/fft.cpp -o test && ./test

int main() {
    OFDM_demod OFDM_demod;

    std::vector<std::complex<double>> vec1 = {{1, 1}, {2, 0}, {3, 0}, {4, 0}};
    std::vector<std::complex<double>> vec2 = {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {1, 1}, {1, 1}, {1, 1}, {1, 1}, {1, 1}, {1, 1}, {1, 1}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}};
    
    auto convolved = OFDM_demod.fft_convolve(vec1, vec2);

    // Печать выходных данных
    for (const auto &val : convolved) {
        std::cout << val << ",\n";
    }

    return 0;
}

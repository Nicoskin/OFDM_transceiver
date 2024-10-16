#include "../ofdm_demod.h"
#include "../fft/fft.h"
#include <iostream>

// g++ demod_fft_conv.cpp ../ofdm_demod.cpp ../fft/fft.cpp -o test && ./test

int main() {
    OFDM_demod OFDM_demod;

    std::vector<std::complex<double>> vec1 = {{1, 1}, {2, 2}, {3, 3}, {4, 4}};
    std::vector<std::complex<double>> vec2 = {{0, 0}, {0, 0}, {4, 4}, {3, 3}, {2, 2}, {0, 0}, {0, 0}, {0, 0},{0, 0}, {1, 1}, {1, 1}, {1, 1}, {2, 2}, {3, 3.2}, {3.5, 3.4}, {1, 1}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}};
    
    std::vector<std::complex<double>> vec1_nsh = {{1, 1}, {2, 2}, {3, 3}, {4, 4}};
    std::vector<std::complex<double>> vec2_nsh = {{0, 0}, {0, 0}, {0, 0}, {0, 0}};
    std::vector<std::complex<double>> vec3_nsh = {{1, 1}, {2, 2}, {3, 3}, {4, 0}};

    // auto corr_n_sh = OFDM_demod.correlate_no_shift(vec1_nsh, vec3_nsh, true);
    // std::cout << "corr_n_sh = " << corr_n_sh << std::endl;
    auto convolved = OFDM_demod.convolve(vec2, vec1);
    auto corr = OFDM_demod.correlateShifted(vec2, vec1, true);
    auto corr_sh = OFDM_demod.correlate(vec2, vec1, false);

    // Печать выходных данных
    for (const auto &val : corr) {
        std::cout << val << ",\n";
    }
     std::cout << "convolved " << std::endl;
    for (const auto &val : convolved) {
        std::cout << val << ",\n";
    }
    std::cout << "corr_sh " << std::endl;
    for (const auto &val : corr_sh) {
        std::cout << val << ",\n";
    }

    return 0;
}

#include "../ofdm_demod.h"
#include "../fft/fft.h"
#include "../ofdm_mod.h"
#include <iostream>

// g++ test_cp_find.cpp ../ofdm_demod.cpp ../fft/fft.cpp ../ofdm_mod.cpp -o test && ./test

int main() {

    OFDM_demod OFDM_demod;
    OFDM_mod OFDM_mod;

    std::vector<std::vector<std::complex<double>>> input_matrix = {
        {{-0.707, 0.707}, {0.707, 0.707}, {0.707, -0.707}, {-0.707, -0.707}, {0.707, 0.707}, {-0.707, 0.707}, {0.707, 0.707}, {-0.707, -0.707}, {-0.707, -0.707}}};

    auto ofdm_sig = OFDM_mod.modulate(input_matrix);

    auto corr = OFDM_demod.CP_CorrIndexs(ofdm_sig);

    // for (auto &val : ofdm_sig) {
    //     std::cout << val << ",\n";
    // }

    for (auto &val : OFDM_mod.data_indices) {
        std::cout << val << " ";
    }

    return 0;
}
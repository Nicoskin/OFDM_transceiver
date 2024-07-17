#include "ofdm_mod.h"
#include "fft/fft.h"
#include <cmath>
#include <complex>

namespace {
    using cd = std::complex<double>;
}

OFDM::OFDM(int N_FFT, int G_subcar, int N_PILOTS, int CP_LEN)
    : N_FFT(N_FFT), G_subcar(G_subcar), N_PILOTS(N_PILOTS), CP_LEN(CP_LEN) {
    N_active_subcarriers = N_FFT - G_subcar - N_PILOTS;
}

std::vector<cd> OFDM::process(const std::vector<std::vector<cd>> &input_matrix, const std::vector<cd> &PSS) {
    
    std::vector<cd> output;

    for (const auto &input_symbols : input_matrix) {
        // Добавьте PSS перед каждым слотом из 5 символов OFDM
        output.insert(output.end(), PSS.begin(), PSS.end());

        // Каждый OFDM символ
        for (size_t i = 0; i < input_symbols.size(); i += N_active_subcarriers * 5) {
            for (int k = 0; k < 5; ++k) {
                // 66 активных поднесущих 
                std::vector<cd> ofdm_symbol(input_symbols.begin() + i + k * N_active_subcarriers,
                                                              input_symbols.begin() + i + (k + 1) * N_active_subcarriers);
                ofdm_symbol = mapToSubcarriers(ofdm_symbol);

                // IFFT
                auto time_domain_symbol = ifft(ofdm_symbol);

                // Add cyclic prefix
                std::vector<cd> cp(time_domain_symbol.end() - CP_LEN, time_domain_symbol.end());
                cp.insert(cp.end(), time_domain_symbol.begin(), time_domain_symbol.end());

                // Add to output
                output.insert(output.end(), cp.begin(), cp.end());
            }
        }
    }

    // Умножение на 2^10
    for (auto &sample : output) {
        sample *= std::pow(2, 0);
    }

    return output;
}

std::vector<cd> OFDM::mapToSubcarriers(const std::vector<cd> &input) {

    std::vector<cd> subcarriers(N_FFT, 0);

    // Сопоставление входных данных с активными поднесущими (при условии симметричного сопоставления)
    int offset = (N_FFT - N_active_subcarriers) / 2;
    for (size_t i = 0; i < input.size(); ++i) {
        subcarriers[offset + i] = input[i];
    }

    return subcarriers;
}

// Последовательность Zadoff-Chu для PSS
// u - 25 29 34
std::vector<cd> OFDM::ZadoffChu(bool PSS, int u, int N) {

    std::vector<cd> zc_sequence;

    if (PSS) {
        N = 63;
        for (int n = 0; n < 31; ++n) {
            zc_sequence.push_back(std::exp(cd(0, -M_PI * u * n * (n + 1) / N)));
        }
        for (int n = 31; n < 62; ++n) {
            zc_sequence.push_back(std::exp(cd(0, -M_PI * u * (n + 1) * (n + 2) / N)));
        }
    } 
    else {
        for (int n = 0; n < N; ++n) {
            zc_sequence.push_back(std::exp(cd(0, -M_PI * u * n * (n + 1) / N)));
        }
    }

    return zc_sequence;
}

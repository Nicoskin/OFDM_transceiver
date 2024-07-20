#include "ofdm_mod.h"
#include "fft/fft.h"
#include <cmath>
#include <complex>
#include <algorithm>

#include <iostream>

namespace {
    using cd = std::complex<double>;
}

OFDM::OFDM(int N_FFT, int G_subcar, int N_PILOTS, int CP_LEN)
    : N_FFT(N_FFT), G_subcar(G_subcar), N_PILOTS(N_PILOTS), CP_LEN(CP_LEN) {
    N_active_subcarriers = N_FFT - G_subcar - N_PILOTS;
    N_active_subcar_for_pss = N_FFT - G_subcar;
}

std::vector<cd> OFDM::process(const std::vector<std::vector<cd>> &input_matrix) {
    std::vector<cd> output;

    for (const auto &input_symbols : input_matrix) {
        // Маппинг и вставка PSS перед каждым слотом из 5 символов OFDM
        auto PSS = ZadoffChu(true);
        auto mapped_pss = mapPSS(PSS);
        output.insert(output.end(), mapped_pss.begin(), mapped_pss.end());
        
        // Каждый OFDM слот
        for (size_t i = 0; i < input_symbols.size(); i += N_active_subcarriers * 5) {
            for (int k = 0; k < 5; ++k) {
                // 66 активных поднесущих 
                std::vector<cd> ofdm_symbol(input_symbols.begin() + i + k * (N_active_subcarriers-1),
                                            input_symbols.begin() + i + (k + 1) * (N_active_subcarriers-1));
                ofdm_symbol = mapToSubcarriers(ofdm_symbol);
        
                // FFTShift
                ofdm_symbol = fftshift(ofdm_symbol);

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
        sample *= std::pow(2, 10);
    }

    return output;
}

std::vector<cd> OFDM::mapToSubcarriers(const std::vector<cd> &input) {

    std::vector<cd> subcarriers(N_FFT, 0);
    int left_active = (N_FFT - N_active_subcarriers) / 2;
    int middle_index = N_FFT / 2;
    int data_index = 0;
    int pilot_index = 0;
    int pilot_interval = N_active_subcarriers / (N_PILOTS-1);
    cd pilot_value(1, 1);  
    // Индексы пилотов - 30 43 56 69 82 95  

    for (int i = 0; i < N_active_subcarriers; ++i) {
        int current_index = left_active + i;

        if (current_index == middle_index) {
            subcarriers[current_index] = cd(0.0, 0.0);
        }

        else if (pilot_index < N_PILOTS && i % pilot_interval == 0) {
            subcarriers[current_index] = pilot_value;
            pilot_index++;
        } else {
            subcarriers[current_index] = input[data_index];
            data_index++;
        }
    }
    return subcarriers;
}
// Маппинг PSS к поднесущим
std::vector<cd> OFDM::mapPSS(const std::vector<cd> &pss) {
    std::vector<cd> subcarriers(N_FFT, 0);

    int left_active = (N_FFT - 63) / 2; //+ (N_FFT - N_active_subcar_for_pss) - 31 ;
    int right_active = left_active + N_active_subcarriers;
    int max_i = left_active - 31;

    for (int i = 0; i < 63; ++i) {
        subcarriers[left_active + i] = pss[i];
        if (i==32) subcarriers[left_active + i] = 0;
    }
    // FFTShift
    subcarriers = fftshift(subcarriers);

    // IFFT
    subcarriers = ifft(subcarriers);

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
    } else {
        for (int n = 0; n < N; ++n) {
            zc_sequence.push_back(std::exp(cd(0, -M_PI * u * n * (n + 1) / N)));
        }
    }

    return zc_sequence;
}

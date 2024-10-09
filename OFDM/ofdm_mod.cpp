#include "ofdm_mod.h"
#include "fft/fft.h"
#include <cmath>
#include <complex>
#include <algorithm>

#include <iostream>

namespace {
    using cd = std::complex<double>;
}

// std::cout << "ofdm_symbol" << std::endl;
// for (const auto &val : ofdm_symbol) {
//     std::cout << val << ",\n";
// }

OFDM_mod::OFDM_mod(){
    N_active_subcarriers = N_FFT - G_SUBCAR; // - N_PILOTS; 
    generateIndices();  // Генерируем индексы данных и пилотов при инициализации
};


// OFDM модуляция
// На вход матрица IQ символов, на выходе сэмплы для передачи
std::vector<cd> OFDM_mod::modulate(const std::vector<std::vector<cd>> &input_matrix) {
    std::vector<cd> output;

    auto mapped_pss = mapPSS();
    // Резервируем место для циклического префикса и символа
    mapped_pss.insert(mapped_pss.begin(), mapped_pss.end() - CP_LEN, mapped_pss.end());

    for (const auto &input_symbols : input_matrix) {

        // Вставка PSS перед каждым слотом из 5 символов OFDM
        output.insert(output.end(), mapped_pss.begin(), mapped_pss.end());
        
        // Каждый OFDM слот
        for (size_t i = 0; i < input_symbols.size(); i += N_active_subcarriers * OFDM_SYM_IN_SLOT) {

            for (int k = 0; k < OFDM_SYM_IN_SLOT; ++k) {
                // input_symbols - данные на 1 символ
                std::vector<cd> ofdm_symbol(input_symbols.begin() + i +  k      * (N_active_subcarriers-1),
                                            input_symbols.begin() + i + (k + 1) * (N_active_subcarriers-1));

                ofdm_symbol = mapToSubcarriers(ofdm_symbol);

                ofdm_symbol = fftshift(ofdm_symbol);

                auto time_domain_symbol = ifft(ofdm_symbol);

                // Добавление циклического префикса
                std::vector<cd> cp(time_domain_symbol.end() - CP_LEN, time_domain_symbol.end());
                cp.insert(cp.end(), time_domain_symbol.begin(), time_domain_symbol.end());

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

// Маппинг данных по поднесущим
std::vector<cd> OFDM_mod::mapToSubcarriers(const std::vector<cd> &input) {
    std::vector<cd> subcarriers(N_FFT, 0);

    cd pilot_value(1, 1);
    
    // Расставляем пилоты по заранее известным индексам
    for (int pilot_index : pilot_indices) {
        subcarriers[pilot_index] = pilot_value;
    }

    // Расставляем данные по заранее известным индексам
    int data_index = 0;
    for (int data_pos : data_indices) {
        subcarriers[data_pos] = input[data_index++];
    }

    return subcarriers;
}

// Маппинг PSS, ifft над PSS
std::vector<cd> OFDM_mod::mapPSS() {
    std::vector<cd> subcarriers(N_FFT, 0);
    std::vector<cd> pss = ZadoffChu();

    int left_active = (N_FFT / 2) - 31;

    for (int i = 0; i < 62; ++i) {
        subcarriers[left_active + i] = pss[i];
        if (i==31) subcarriers[left_active + i] = 0;
    }

    subcarriers = fftshift(subcarriers);

    subcarriers = ifft(subcarriers);

    return subcarriers;
}

void OFDM_mod::generateIndices() {
    data_indices.clear();
    pilot_indices.clear();

    int left_active = G_SUBCAR / 2;
    int middle_index = N_FFT / 2;
    int pilot_interval = N_active_subcarriers / (N_PILOTS - 1);

    for (int i = 0; i < N_active_subcarriers; ++i) {
        int current_index = left_active + i;

        if (current_index == middle_index-1) {
            continue; // Пропускаем DC-компонент
        } else if (i % pilot_interval == 0 && pilot_indices.size() < N_PILOTS) {
            // Генерируем индексы для пилотов
            pilot_indices.push_back(current_index);
        } else {
            // Генерируем индексы для данных
            data_indices.push_back(current_index);
        }
    }
}

// Последовательность Zadoff-Chu для PSS
// u - 25 29 34
std::vector<cd> OFDM_mod::ZadoffChu(int u) {
    std::vector<cd> zc_sequence;

    std::complex<double> j(0, 1);
    double epsilon = 0.001; // пороговое значение для мнимой части
    int N = 63;

    for (int n = 0;  n <= 30; n++) {
        cd zc_value = exp(-j * M_PI * cd(u) * cd(n)     * cd(n + 1) / cd(N));
        // Проверка мнимой части
        // Если мнимая часть меньше epsilon, установить её в 0
        if (std::abs(imag(zc_value)) < epsilon) {zc_value = cd(real(zc_value), 0.0);} 
        zc_sequence.push_back(zc_value);
    }

    for (int n = 31; n <= 61; n++) {
        cd zc_value = exp(-j * M_PI * cd(u) * cd(n + 1) * cd(n + 2) / cd(N));
        if (std::abs(imag(zc_value)) < epsilon) {zc_value = cd(real(zc_value), 0.0);}
        zc_sequence.push_back(zc_value);
    }

    return zc_sequence;
}

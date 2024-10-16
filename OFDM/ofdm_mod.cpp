#include "ofdm_mod.h"
#include "fft/fft.h"

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
        subcarriers[data_pos]    = input[data_index++];
    }

    return subcarriers;
}

// Маппинг PSS, ifft над PSS
std::vector<cd> OFDM_mod::mapPSS(int u) {
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

// Маппинг PSS, ifft над PSS
std::vector<cd> OFDM_mod::mapSSS(int N_ID_cell) {
    std::vector<cd> subcarriers(N_FFT, 0);
    auto sss = generate_sss(N_ID_cell);

    int left_active = (N_FFT / 2) - 31;

    for (int i = 0; i < 62; ++i) {
        subcarriers[left_active + i] = sss[i];
        if (i==31) subcarriers[left_active + i] = 0;
    }

    subcarriers = fftshift(subcarriers);

    subcarriers = ifft(subcarriers);

    return subcarriers;
}

// Функция для генерации SSS
std::vector<cd> OFDM_mod::generate_sss(int N_ID_cell) {
    // Шаг 2: Вычисление промежуточных значений
    int q_prime = N_ID_cell / 30;
    int q = (N_ID_cell + q_prime * (q_prime + 1) / 2) / 30;
    int m_prime = N_ID_cell + q * (q + 1) / 2;
    int m0 = m_prime % 31;
    int m1 = (m0 + (m_prime / 31) + 1) % 31;
    
    //std::cout << "m0: " << m0 << " m1: " << m1 << std::endl;

    // Шаг 3: Генерация последовательности x_s для s_tilda
    std::vector<int8_t> x_s(31, 0);
    x_s[4] = 1;  // начальные значения: x_s(1:5) = [0 0 0 0 1]
    for (int i = 0; i < 26; ++i) {
        x_s[i + 5] = (x_s[i + 2] + x_s[i]) % 2;
    }

    // Шаг 4: Генерация последовательности x_c для c_tilda
    std::vector<int8_t> x_c(31, 0);
    x_c[4] = 1;  // начальные значения: x_c(1:5) = [0 0 0 0 1]
    for (int i = 0; i < 26; ++i) {
        x_c[i + 5] = (x_c[i + 3] + x_c[i]) % 2;
    }

    // Шаг 5: Генерация последовательности x_z для z_tilda
    std::vector<int8_t> x_z(31, 0);
    x_z[4] = 1;  // начальные значения: x_z(1:5) = [0 0 0 0 1]
    for (int i = 0; i < 26; ++i) {
        x_z[i + 5] = (x_z[i + 4] + x_z[i + 2] + x_z[i + 1] + x_z[i]) % 2;
    }

    // Шаг 6: Генерация последовательностей s_tilda, c_tilda и z_tilda
    std::vector<int8_t> s_tilda(31, 0);
    std::vector<int8_t> c_tilda(31, 0);
    std::vector<int8_t> z_tilda(31, 0);
    for (int i = 0; i < 31; ++i) {
        s_tilda[i] = 1 - 2 * x_s[i];
        c_tilda[i] = 1 - 2 * x_c[i];
        z_tilda[i] = 1 - 2 * x_z[i];
    }

    // Шаг 7: Генерация последовательности s0_m0_even
    std::vector<int8_t> s0_m0_even(31, 0);
    for (int n = 0; n < 31; ++n) {
        s0_m0_even[n] = s_tilda[(n + m0) % 31];
    }

    // Шаг 8: Генерация последовательности c0_even
    std::vector<int8_t> c0_even(31, 0);
    for (int n = 0; n < 31; ++n) {
        c0_even[n] = c_tilda[(n + N_ID_cell) % 31];
    }

    // Шаг 9: Вычисление d_even_sub0
    std::vector<int8_t> d_even_sub0(31, 0);
    for (int n = 0; n < 31; ++n) {
        d_even_sub0[n] = s0_m0_even[n] * c0_even[n];
    }

    // Шаг 10: Генерация последовательности s1_m1_odd
    std::vector<int8_t> s1_m1_odd(31, 0);
    for (int n = 0; n < 31; ++n) {
        s1_m1_odd[n] = s_tilda[(n + m1) % 31];
    }

    // Шаг 11: Генерация последовательности c1_odd
    std::vector<int8_t> c1_odd(31, 0);
    for (int n = 0; n < 31; ++n) {
        c1_odd[n] = c_tilda[(n + N_ID_cell + 3) % 31];
    }

    // Шаг 12: Генерация последовательности z1_m0_odd
    std::vector<int8_t> z1_m0_odd(31, 0);
    for (int n = 0; n < 31; ++n) {
        z1_m0_odd[n] = z_tilda[(n + (m0 % 8)) % 31];
    }

    // Шаг 13: Вычисление d_odd_sub0
    std::vector<int8_t> d_odd_sub0(31, 0);
    for (int n = 0; n < 31; ++n) {
        d_odd_sub0[n] = s1_m1_odd[n] * c1_odd[n] * z1_m0_odd[n];
    }

    // Шаг 14: Объединение d_even_sub0 и d_odd_sub0 в d_sub0
    std::vector<int8_t> d_sub0(62, 0);
    for (int n = 0; n < 31; ++n) {
        d_sub0[2 * n] = d_even_sub0[n];
        d_sub0[2 * n + 1] = d_odd_sub0[n];
    }

    std::vector<cd> result;
    for (int8_t val : d_sub0) {
        result.emplace_back(static_cast<double>(val), 0);
    }

    return result;
}
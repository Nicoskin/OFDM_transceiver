#include "sequence.h"
#include <iostream>
#include <cstdint>


void gen_pilots_siq(std::vector<std::vector<std::vector<cd>>>& refs, int N_cell, bool amp_pilots_high) {
    size_t num_slots = 20;
    size_t num_symbols = 7;
    uint16_t N_rb_max = 110;
    uint16_t N_rb = ((N_FFT - G_SUBCAR - 1) / 12); // 6
    size_t num_pilots = N_rb * 2;
    
    // Изменяем размер матрицы c на 20x7x440
    std::vector<std::vector<std::vector<int32_t>>> c(num_slots, std::vector<std::vector<int32_t>>(num_symbols, std::vector<int32_t>(N_rb_max*2*2, 0)));
    std::vector<std::vector<std::vector<cd>>> r(num_slots, std::vector<std::vector<cd>>(num_symbols, std::vector<cd>(N_rb_max*2, 0)));

    uint16_t N_cp = !CP_LEN; // 1-normal CP, 0-extended CP 

    for (size_t ns = 0; ns < num_slots; ns++) {
        for (size_t l = 0; l < num_symbols; l++) {
            int c_init = 1024 * (7 * (ns + 1) + l + 1) * (2 * N_cell + 1) + 2 * N_cell + N_cp;

            int N_c = 1600;
            int Mpn = 2 * 2 * N_rb_max;

            std::vector<int8_t> x_1(N_c + Mpn + 31, 0);
            x_1[0] = 1;
            for (int n = 0; n < N_c + Mpn; n++) {
                x_1[n + 31] = (x_1[n + 3] + x_1[n]) % 2;
            }

            std::vector<int8_t> x_2(N_c + Mpn + 31, 0);
            for (int i = 0; i < 30; ++i) {
                x_2[i] = (c_init >> i) & 1;
            }
            for (int n = 0; n < N_c + Mpn; n++) {
                x_2[n + 31] = (x_2[n + 3] + x_2[n + 2] + x_2[n + 1] + x_2[n]) % 2;  
            }

            // Заполнение c[ns][l][i] значениями
            for (size_t i = 0; i < N_rb_max * 2 * 2; i++) { // 440
                c[ns][l][i] = (x_1[i + N_c] + x_2[i + N_c]) % 2;
            }
            // if ((ns == 0) && (l == 0)) {
            //     std::cout << "c_init = " << c_init << "   cell.id = " << N_cell << std::endl;
            //     for (size_t i = 208; i < 232; i++) {
            //         std::cout << c[ns][l][i] << " ";
            //     }
            //     std::cout << std::endl;
            // }
            // std::cout << "ns = " << ns << "   l = " << l << std::endl;
            // std::cout << "c_init = " << c_init << "   cell.id = " << N_cell << std::endl;
            // for (size_t i = 0; i < num_pilots * 2; i++) {
            //     std::cout << c[ns][l][i] << " ";
            // }
            // std::cout << std::endl;
            
        }
    }

    double multiplier = (1 / sqrt(2.0));
    if (amp_pilots_high) {multiplier = 1;}

    int mp;
    for (size_t ns = 0; ns < num_slots; ns++) {
        for (size_t l = 0; l < num_symbols; l++) {
            for (size_t m = 0; m < N_rb*2; m++) { // 12
                mp = m + N_rb_max - N_rb;
                //std::cout << "mp: " << mp << std::endl;
                refs[ns][l][m] = {
                    multiplier * (1 - 2 * c[ns][l][2 * mp + 0]),
                    multiplier * (1 - 2 * c[ns][l][2 * mp + 1])};
            }
        }
    }


    // for (size_t ns = 0; ns < num_slots; ns++) {
    //     for (size_t l = 0; l < num_symbols; l++) {
    //         std::cout << "refs[" << ns << "][" << l << "]: ";
    //         for (size_t i = 0; i < num_pilots*2; i++) {
    //             std::cout << c[ns][l][i] << " ";
    //         }
    //         std::cout << std::endl;
    //     }
    // }

}


// Последовательность Zadoff-Chu для PSS
// u - 25 29 34
std::vector<cd> ZadoffChu(int u) {
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

// Функция для генерации SSS
std::vector<cd> generate_sss(int N_ID_cell, int subframe, bool add_zero_middle) {
    if (subframe != 0 && subframe != 5) {
        throw std::invalid_argument("Subframe must be 0 or 5");
    }
    int N_ID_1 = (N_ID_cell - (N_ID_cell % 3)) / 3;
    int N_ID_2 = N_ID_cell % 3;
    // std::cout << "N_ID_1: " << N_ID_1 << " N_ID_2: " << N_ID_2 << std::endl;

    // Шаг 2: Вычисление промежуточных значений
    int q_prime = N_ID_1 / 30;
    int q = (N_ID_1 + q_prime * (q_prime + 1) / 2) / 30;
    int m_prime = N_ID_1 + q * (q + 1) / 2;
    int m0 = m_prime % 31;
    int m1 = (m0 + (m_prime / 31) + 1) % 31;

    //std::cout << "m0: " << m0 << " m1: " << m1 << std::endl;

    // Шаг 3: Генерация последовательности x_s для s_tilda
    std::vector<int8_t> x_s(31, 0);
    x_s[4] = 1;  // начальные значения: x_s(1:5) = [0 0 0 0 1]
    for (int i = 0; i <= 25; ++i) {
        x_s[i + 5] = (x_s[i + 2] + x_s[i]) % 2;
    }

    // Шаг 4: Генерация последовательности x_c для c_tilda
    std::vector<int8_t> x_c(31, 0);
    x_c[4] = 1;  // начальные значения: x_c(1:5) = [0 0 0 0 1]
    for (int i = 0; i <= 25; ++i) {
        x_c[i + 5] = (x_c[i + 3] + x_c[i]) % 2;
    }

    // Шаг 5: Генерация последовательности x_z для z_tilda
    std::vector<int8_t> x_z(31, 0);
    x_z[4] = 1;  // начальные значения: x_z(1:5) = [0 0 0 0 1]
    for (int i = 0; i <= 25; ++i) {
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

    // Шаг 7: Генерация последовательности s0_m0_even s1_m1_even
    std::vector<int8_t> s0_m0_n(31, 0);
    for (int n = 0; n < 31; ++n) {
        s0_m0_n[n] = s_tilda[(n + m0) % 31];
    }
    std::vector<int8_t> s1_m1_n(31, 0);
    for (int n = 0; n < 31; ++n) {
        s1_m1_n [n] = s_tilda[(n + m1) % 31];
    }

    std::vector<int8_t> c0_n(31, 0);
    for (int n = 0; n < 31; ++n) {
        c0_n[n] = c_tilda[(n + N_ID_2) % 31];
    }
    std::vector<int8_t> c1_n(31, 0);
    for (int n = 0; n < 31; ++n) {
        c1_n[n] = c_tilda[(n + N_ID_2 + 3) % 31];
    }


    std::vector<int8_t> z1_m0_n(31, 0);
    for (int n = 0; n < 31; ++n) {
        z1_m0_n[n] = z_tilda[(n + (m0 % 8)) % 31];
    }
    std::vector<int8_t> z1_m1_n(31, 0);
    for (int n = 0; n < 31; ++n) {
        z1_m1_n[n] = z_tilda[(n + (m1 % 8)) % 31];
    }

    std::vector<int8_t> d_2n_sub0(62, 0);
    for(int n = 0; n < 31; ++n) {
        d_2n_sub0[2 * n] = s0_m0_n[n] * c0_n[n];
        d_2n_sub0[2 * n + 1] = s1_m1_n[n] * c1_n[n] * z1_m0_n[n];
    }

    std::vector<int8_t> d_2n_sub5(62, 0);
    for(int n = 0; n < 31; ++n) {
        d_2n_sub5[2 * n] = s1_m1_n[n] * c0_n[n];
        d_2n_sub5[2 * n + 1] = s0_m0_n[n] * c1_n[n] * z1_m1_n[n];
    }

    std::vector<cd> result;
    if (subframe == 0) {
        for (int8_t val : d_2n_sub0) {
            result.emplace_back(static_cast<double>(val), 0);
        }
    } else {
        for (int8_t val : d_2n_sub5) {
            result.emplace_back(static_cast<double>(val), 0);
        }
    }
    
    if (add_zero_middle) {
        result.insert(result.begin() + result.size() / 2, (0, 0));
    }

    return result;
}
#include "pbch.h"
#include <cmath>
#include <algorithm>
#include <limits>

//
// Scrambling
//
std::vector<uint8_t> scrambling(std::vector<uint8_t>bits, int N_cell, int frame){
    uint16_t N_rb_max = 110;
    int nf = 0; // nf mod 4 = 0
    int c_init = N_cell;
    int size = 1920;
    frame = frame * (size / 4);

    int N_c = 1600;
    int Mpn = 2 * 2 * N_rb_max;
    Mpn = size;

    std::vector<int8_t> x_1(N_c + Mpn + 31, 0);
    x_1[0] = 1;
    for (int n = 0; n < N_c + Mpn; n++) {
        x_1[n + 31] = (x_1[n + 3] ^ x_1[n]);
    }

    std::vector<int8_t> x_2(N_c + Mpn + 31, 0);
    for (int i = 0; i < 30; ++i) {
        x_2[i] = (c_init >> i) & 1;
    }
    for (int n = 0; n < N_c + Mpn; n++) {
        x_2[n + 31] = (x_2[n + 3] ^ x_2[n + 2] ^ x_2[n + 1] ^ x_2[n]);  
    }

    std::vector<int8_t> c(size, 0);
    for (size_t i = 0; i < size; i++) { // 440
        c[i] = (x_1[i + N_c] ^ x_2[i + N_c]);
    }
    //std::cout << "frame: " << frame << " - " << frame + bits.size() << std::endl;
    std::vector<uint8_t> scrambled_bits;
    for(size_t i = 0; i < bits.size(); i++){
        scrambled_bits.push_back(bits[i] ^ c[i + frame]);
    }
    return scrambled_bits;
}


//
// RM
//
std::vector<int> _rate_match(const std::vector<int> &d, int E) {
    #define RM_NULL 8

    constexpr int C_CC_subblock = 32;
    const std::vector<int> P = {1, 17, 9, 25, 5, 21, 13, 29, 3, 19, 11, 27, 7, 23, 15, 31,
                                0, 16, 8, 24, 4, 20, 12, 28, 2, 18, 10, 26, 6, 22, 14, 30};
    int D = d.size() / 3; 
    int R_CC_subblock = D / C_CC_subblock + 1;
    int matrix_size = R_CC_subblock * C_CC_subblock;
    int N_D = matrix_size - D;

    std::vector<std::vector<int>> v(d.size());
    for (size_t stream = 0; stream < 3; ++stream) {
        // Step 3: Padding with NULL values
        std::vector<std::vector<int>> interleave_matrix(R_CC_subblock, std::vector<int>(C_CC_subblock, -1));
        int idx = 0;
        std::cout << stream << std::endl;
        std::cout << "interleave_matrix:\n";
        for (int i = 0; i < R_CC_subblock; i++) {
            for (int j = 0; j < C_CC_subblock; j++) {
                if (idx >= N_D) interleave_matrix[i][j] = d[(idx - N_D) * 3 + stream];
                else interleave_matrix[i][j] = RM_NULL;
                idx++;
                std::cout << interleave_matrix[i][j] << "";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;

        // Step 4: Column permutation
        std::vector<std::vector<int>> permuted_matrix(R_CC_subblock, std::vector<int>(C_CC_subblock));
        for (int j = 0; j < C_CC_subblock; j++) {
            for (int i = 0; i < R_CC_subblock; i++) {
                permuted_matrix[i][j] = interleave_matrix[i][P[j]];
            }
        }
        std::cout << "permuted_matrix:\n";
        for (int i = 0; i < R_CC_subblock; i++) {
            for (int j = 0; j < C_CC_subblock; j++) {
                std::cout << permuted_matrix[i][j] << "";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;


        // Step 5: Read column-wise to get v(i)_k
        for (int j = 0; j < C_CC_subblock; j++) {
            for (int i = 0; i < R_CC_subblock; i++) {
                v[stream].push_back(permuted_matrix[i][j]);
            }
        }

    }

    // std::cout << "v[0]: ";
    // for (int bit : v[0]) std::cout << bit ;
    // std::cout << std::endl;
    // std::cout << "v[1]: ";
    // for (int bit : v[1]) std::cout << bit ;
    // std::cout << std::endl;
    // std::cout << "v[2]: ";
    // for (int bit : v[2]) std::cout << bit ;
    // std::cout << std::endl;

    // Bit collection
    int K_Pi = v[0].size();
    std::vector<int> w(3 * K_Pi);
    for (size_t i = 0; i < K_Pi; i++) {
        w[i] = v[0][i];
        w[K_Pi + i] = v[1][i];
        w[2 * K_Pi + i] = v[2][i];
    }
    // std::cout << "w: (" << w.size() << ")\n";
    // for (int bit : w) std::cout << bit ;
    // std::cout << std::endl;

    // std::cout << "w: (" << w.size() << ")\n";
    // for (int bit : w) if (bit != RM_NULL) std::cout << bit ;
    // std::cout << std::endl;


    // Rate matching selection
    std::vector<int> e;
    int k = 0, j = 0;
    while (k < E) {
        if (w[j] != RM_NULL) {
            e.push_back(w[j]);
            k++;
        }
        j++;
        if ( j == 3 * K_Pi) j = 0;
    }

    return e;
}

std::vector<int> rate_match(const std::vector<int> &d, int out_len) {
    #define RM_NULL 8

    constexpr int C_CC_subblock = 32;
    const std::vector<int> P = {1, 17, 9, 25, 5, 21, 13, 29, 3, 19, 11, 27, 7, 23, 15, 31,
                                0, 16, 8, 24, 4, 20, 12, 28, 2, 18, 10, 26, 6, 22, 14, 30};
    int D = d.size()/3; 
    int R_CC_subblock = (D + C_CC_subblock - 1) / C_CC_subblock;
    int matrix_size = R_CC_subblock * C_CC_subblock; //K_p from srsran
    int N_D = matrix_size - D;

    int k = 0;
    std::vector<int> tmp(R_CC_subblock * C_CC_subblock * 3);
    for (size_t stream = 0; stream < 3; stream++){
        for (int j = 0; j < C_CC_subblock; j++){
            for (int i = 0; i < R_CC_subblock; i++){
                if (i * C_CC_subblock + P[j] < N_D) {
                    tmp[k] = RM_NULL;
                } else {
                    tmp[k] = d[(i * C_CC_subblock + P[j] - N_D) * 3 + stream];
                }
                k++;
            }
        }
    }

    // Rate matching selection
    std::vector<int> e;
    k = 0;
    int j = 0;
    while (k < out_len) {
        if (tmp[j] != RM_NULL) {
            e.push_back(tmp[j]);
            k++;
        }
        j++;
        if ( j == 3 * matrix_size) 
            j = 0;
    }

    return e;
}


std::vector<uint8_t> de_rate_match(const std::vector<uint8_t>& e, int D){
    #define RM_NULL 8
    constexpr int C_CC_subblock = 32;
    const std::vector<int> P = {1, 17, 9, 25, 5, 21, 13, 29, 3, 19, 11, 27, 7, 23, 15, 31,
                                0, 16, 8, 24, 4, 20, 12, 28, 2, 18, 10, 26, 6, 22, 14, 30};
                                
    std::vector<std::vector<int>> d_k(3, std::vector<int>());                      
    // Вычисление параметров матрицы
    int R_CC_subblock = (D/3 + C_CC_subblock - 1) / C_CC_subblock;
    int matrix_size = R_CC_subblock * C_CC_subblock;
    int N_D = matrix_size - D/3;
    int K_Pi = D / 3;

    std::vector<int> w_k = std::vector<int>(e.begin(), e.begin() + D); // неповторяющиеся биты
    for (int stream = 0; stream < 3; stream++){
        std::vector<int> w_0 = std::vector<int>(w_k.begin() + stream * K_Pi, w_k.begin() + (stream + 1) * K_Pi); // один поток

        std::vector<std::vector<int>> permuted_matrix(R_CC_subblock, std::vector<int>(C_CC_subblock));

        int idx = 0;
        //std::cout << idx << " " << w_0.size() << std::endl;
        for (int j = 0; j < C_CC_subblock; j++){
            if (P[j] < N_D){
                permuted_matrix[0][P[j]] = RM_NULL;
                permuted_matrix[1][P[j]] = w_0[idx++];
            }
            else{
                permuted_matrix[0][P[j]] = w_0[idx];
                permuted_matrix[1][P[j]] = w_0[++idx];
                idx++;
            }
        }

        // std::cout << stream <<std::endl;
        // for (int i = 0; i < R_CC_subblock; i++) {
        //     for (int j = 0; j < C_CC_subblock; j++) {
        //         std::cout << permuted_matrix[i][j];
        //     }
        //     std::cout << std::endl;
        // }
        // std::cout << std::endl;
        for (int i = 0; i < R_CC_subblock; i++) {
            for (int j = 0; j < C_CC_subblock; j++) {
                if (((i == 0) && (j >= N_D)) || (i == 1)){
                    d_k[stream].push_back(permuted_matrix[i][j]);
                }
            }
        }
        //std::cout << "d_k[" << stream << "]: " << d_k[stream].size() << std::endl;
    }
    std::vector<uint8_t> d;
    for (int i = 0; i < K_Pi; i++){
        for (int j = 0; j < 3; j++){
            d.push_back(d_k[j][i]);
        }
    }
    return d;


}


//
// CRC
//
bool check_crc(const std::vector<uint8_t> &c){
    if (c.size() != 40) return -1;
    // Полином CRC-16: x^16 + x^12 + x^5 + 1 (0x1021)
    const uint16_t polynomial = 0x1021;
    uint16_t crcReg = 0x0000; // Регистр CRC

    // Размер данных без CRC (A = bitData.size() - 16)
    const size_t dataBits = c.size() - 16;

    // Обработка входных битов (данные + CRC)
    for (size_t i = 0; i < c.size(); ++i) {
        // Выходной бит (старший бит регистра)
        uint8_t msb = (crcReg >> 15) & 1;

        // Сдвиг регистра влево
        crcReg <<= 1;

        // Добавляем текущий бит данных в младший бит регистра
        crcReg |= (c[i] & 1);

        // Если старший бит до сдвига был 1, выполняем XOR с полиномом
        if (msb) {
            crcReg ^= polynomial;
        }
    }
    // Если остаток равен 0, CRC верен
    return (crcReg == 0);
}

std::vector<uint8_t> calculate_crc_16(const std::vector<uint8_t>& bitData) {
    const uint16_t polynomial = 0x1021; // Полином: x^16 + x^12 + x^5 + 1
    uint16_t crcReg = 0x0000;          // Инициализация регистра CRC

    // Обработка каждого бита входных данных
    for (uint8_t bit : bitData) {
        // Вычисляем значение старшего бита (перед сдвигом)
        uint8_t msb = (crcReg >> 15) & 1;

        // Сдвигаем регистр влево на 1 бит
        crcReg <<= 1;

        // Добавляем текущий бит данных в младший бит регистра
        crcReg |= (bit & 1);

        // Если старший бит был 1, выполняем XOR с полиномом
        if (msb) {
            crcReg ^= polynomial;
        }
    }
    // Преобразуем 16-битный CRC в вектор битов (старший бит идет первым)
    std::vector<uint8_t> crcBits;
    for (int i = 15; i >= 0; --i) {
        crcBits.push_back((crcReg >> i) & 1);
    }

    return crcBits;
}


//
// Coders
//
std::vector<int> encode(const std::vector<int>& input) {
    const std::vector<int> GENERATORS = {0133, 0171, 0165};
    const int CONSTRAINT_LENGTH = 7;

    std::vector<int> output(input.size() * GENERATORS.size(), 0);
    std::vector<int> shift_register(CONSTRAINT_LENGTH, 0);
    
    for (size_t i = 0; i < input.size(); ++i) {
        std::copy_backward(shift_register.begin(), shift_register.end() - 1, shift_register.end());
        shift_register[0] = (i < input.size()) ? input[i] : 0;
        
        for (size_t g = 0; g < GENERATORS.size(); ++g) {
            int encoded_bit = 0;
            for (size_t j = 0; j < CONSTRAINT_LENGTH; ++j) {
                if ((GENERATORS[g] >> (CONSTRAINT_LENGTH - j - 1)) & 1) { 
                    encoded_bit ^= shift_register[j];
                }
            }
            output[i * GENERATORS.size() + g] = encoded_bit;
        }
    }
    return output;
}

std::vector<uint8_t> decode(const std::vector<uint8_t>& input) {
    const std::vector<int> GENERATORS = {0133, 0171, 0165};
    const int CONSTRAINT_LENGTH = 7;
    const int NUM_STATES = 1 << (CONSTRAINT_LENGTH - 1); // 64 состояния
    const int RATE = GENERATORS.size(); // 1/3

    if (input.size() % RATE != 0) {
        return std::vector<uint8_t>(); // Ошибка: некорректная длина
    }

    int input_length = input.size() / RATE;
    std::vector<std::vector<int>> trellis(NUM_STATES, std::vector<int>(input_length + 1, 0));
    std::vector<std::vector<int>> path(NUM_STATES, std::vector<int>(input_length + 1, 0));
    std::vector<std::vector<int>> metrics(NUM_STATES, std::vector<int>(input_length + 1, std::numeric_limits<int>::max()));

    metrics[0][0] = 0;

    // Прямой проход
    for (int t = 0; t < input_length; ++t) {
        std::vector<int> received_bits = {input[t * RATE], input[t * RATE + 1], input[t * RATE + 2]};

        for (int state = 0; state < NUM_STATES; ++state) {
            if (metrics[state][t] == std::numeric_limits<int>::max()) continue;

            for (int input_bit = 0; input_bit < 2; ++input_bit) {
                int next_state = ((state << 1) | input_bit) & (NUM_STATES - 1);
                std::vector<int> expected_output(RATE, 0);

                int shift_reg = (state << 1) | input_bit;
                for (int g = 0; g < RATE; ++g) {
                    int encoded_bit = 0;
                    for (int j = 0; j < CONSTRAINT_LENGTH; ++j) {
                        if ((GENERATORS[g] >> (CONSTRAINT_LENGTH - j - 1)) & 1) {
                            encoded_bit ^= (shift_reg >> j) & 1;
                        }
                    }
                    expected_output[g] = encoded_bit;
                }

                int distance = 0;
                for (int i = 0; i < RATE; ++i) {
                    distance += (received_bits[i] != expected_output[i]);
                }

                int new_metric = metrics[state][t] + distance;
                if (new_metric < metrics[next_state][t + 1]) {
                    metrics[next_state][t + 1] = new_metric;
                    trellis[next_state][t + 1] = state;
                    path[next_state][t + 1] = input_bit;
                }
            }
        }
    }

    // Найти состояние с минимальной метрикой на последнем шаге
    int final_state = 0;
    int min_metric = std::numeric_limits<int>::max();
    for (int state = 0; state < NUM_STATES; ++state) {
        if (metrics[state][input_length] < min_metric) {
            min_metric = metrics[state][input_length];
            final_state = state;
        }
    }

    // Обратный проход
    std::vector<uint8_t> decoded(input_length);
    for (int t = input_length; t > 0; --t) {
        decoded[t - 1] = path[final_state][t];
        final_state = trellis[final_state][t];
    }

    return decoded;
}

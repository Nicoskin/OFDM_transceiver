#include "pbch.h"
#include <cmath>
#include <algorithm>
#include <limits>

std::vector<uint8_t> scrambling(std::vector<uint8_t>bits, int N_cell){
    uint16_t N_rb_max = 110;
    int nf = 0; // nf mod 4 = 0
    int c_init = N_cell;

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

    std::vector<int8_t> c(N_c + Mpn + 31, 0);
    for (size_t i = 0; i < N_rb_max * 2 * 2; i++) { // 440
        c[i] = (x_1[i + N_c] + x_2[i + N_c]) % 2;
    }

    std::vector<uint8_t> scrambled_bits;
    for(size_t i = 0; i < bits.size(); i++){
        scrambled_bits.push_back(bits[i] ^ c[i]);
    }
    return scrambled_bits;
}


std::vector<int> rate_dematching(const std::vector<int>& received, int D) {
    const int C_CC_subblock = 32;
    const int interleaver_permutation[C_CC_subblock] = {1, 17, 9, 25, 5, 21, 13, 29, 3, 19, 11, 27, 7, 23, 15, 31,
                                                       0, 16, 8, 24, 4, 20, 12, 28, 2, 18, 10, 26, 6, 22, 14, 30};
    
    int R_CC_subblock = (D + C_CC_subblock - 1) / C_CC_subblock;
    int total_bits = R_CC_subblock * C_CC_subblock;
    int K_Pi = total_bits;
    int K_w = 3 * K_Pi;

    // Восстановление циклического буфера
    std::vector<float> soft_buffer(K_w, 0.0f);
    int j = 0;
    for (size_t k = 0; k < received.size(); k++) {
        while (true) {
            if (j % K_w < K_w) {
                soft_buffer[j % K_w] += received[k]; // Суммируем повторные вхождения
                j++;
                break;
            }
            j++;
        }
    }

    // Разделение на три потока
    std::vector<std::vector<int>> decoded(3, std::vector<int>(K_Pi));
    for (int i = 0; i < K_Pi; i++) {
        decoded[0][i] = (soft_buffer[i] > 0) ? 1 : 0;
        decoded[1][i] = (soft_buffer[K_Pi + i] > 0) ? 1 : 0;
        decoded[2][i] = (soft_buffer[2*K_Pi + i] > 0) ? 1 : 0;
    }
    std::vector<int> result;
    for (int i = 0; i < D+6; i++) {
        result.push_back(decoded[0][i]);
        result.push_back(decoded[1][i]);
        result.push_back(decoded[2][i]);
    }

    return result;
}



std::vector<int> _rate_match(const std::vector<std::vector<int>> &d, int E) {
    #define RM_NULL 8

    constexpr int C_CC_subblock = 32;
    const std::vector<int> P = {1, 17, 9, 25, 5, 21, 13, 29, 3, 19, 11, 27, 7, 23, 15, 31,
                                0, 16, 8, 24, 4, 20, 12, 28, 2, 18, 10, 26, 6, 22, 14, 30};
    int D = d[0].size(); 
    int R_CC_subblock = D / C_CC_subblock + 1;
    int matrix_size = R_CC_subblock * C_CC_subblock;
    int N_D = matrix_size - D;

    std::vector<std::vector<int>> v(d.size());
    for (size_t stream = 0; stream < d.size(); ++stream) {
        // Step 3: Padding with NULL values
        std::vector<std::vector<int>> interleave_matrix(R_CC_subblock, std::vector<int>(C_CC_subblock, -1));
        int idx = 0;
        std::cout << stream << std::endl;
        std::cout << "interleave_matrix:\n";
        for (int i = 0; i < R_CC_subblock; i++) {
            for (int j = 0; j < C_CC_subblock; j++) {
                if (idx >= N_D) interleave_matrix[i][j] = d[stream][idx - N_D];
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

std::vector<int> rate_match(const std::vector<std::vector<int>> &d, int out_len) {
    #define RM_NULL 8

    constexpr int C_CC_subblock = 32;
    const std::vector<int> P = {1, 17, 9, 25, 5, 21, 13, 29, 3, 19, 11, 27, 7, 23, 15, 31,
                                0, 16, 8, 24, 4, 20, 12, 28, 2, 18, 10, 26, 6, 22, 14, 30};
    int D = d[0].size(); 
    int R_CC_subblock = (D + C_CC_subblock - 1) / C_CC_subblock;
    int matrix_size = R_CC_subblock * C_CC_subblock; //K_p from srsran
    int N_D = matrix_size - D;

    std::vector<int> dd;
    for (size_t i = 0; i < d[0].size(); ++i) {
        for (size_t g = 0; g < d.size(); ++g) {
            dd.push_back(d[g][i]);
        }
    }   

    int k = 0;
    std::vector<int> tmp(R_CC_subblock * C_CC_subblock * 3);
    for (size_t stream = 0; stream < d.size(); stream++){
        for (int j = 0; j < C_CC_subblock; j++){
            for (int i = 0; i < R_CC_subblock; i++){
                if (i * C_CC_subblock + P[j] < N_D) {
                    tmp[k] = RM_NULL;
                } else {
                    tmp[k] = dd[(i * C_CC_subblock + P[j] - N_D) * 3 + stream];
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



std::vector<int> de_rate_match(const std::vector<int>& e, int D){
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
    std::vector<int> d;
    for (int i = 0; i < K_Pi; i++){
        for (int j = 0; j < 3; j++){
            d.push_back(d_k[j][i]);
        }
    }
    return d;


}
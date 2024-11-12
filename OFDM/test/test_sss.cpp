#include "../sequence.h"
#include <iostream>
#include <complex>

// g++ test_sss.cpp ../sequence.cpp -o test && ./test

// // Функция для генерации SSS
// std::vector<int8_t> generate_sss(int N_ID_cell) {
//     // Шаг 2: Вычисление промежуточных значений
//     int q_prime = N_ID_cell / 30;
//     int q = (N_ID_cell + q_prime * (q_prime + 1) / 2) / 30;
//     int m_prime = N_ID_cell + q * (q + 1) / 2;
//     int m0 = m_prime % 31;
//     int m1 = (m0 + (m_prime / 31) + 1) % 31;
    
//     //std::cout << "m0: " << m0 << " m1: " << m1 << std::endl;

//     // Шаг 3: Генерация последовательности x_s для s_tilda
//     std::vector<int8_t> x_s(31, 0);
//     x_s[4] = 1;  // начальные значения: x_s(1:5) = [0 0 0 0 1]
//     for (int i = 0; i < 26; ++i) {
//         x_s[i + 5] = (x_s[i + 2] + x_s[i]) % 2;
//     }

//     // Шаг 4: Генерация последовательности x_c для c_tilda
//     std::vector<int8_t> x_c(31, 0);
//     x_c[4] = 1;  // начальные значения: x_c(1:5) = [0 0 0 0 1]
//     for (int i = 0; i < 26; ++i) {
//         x_c[i + 5] = (x_c[i + 3] + x_c[i]) % 2;
//     }

//     // Шаг 5: Генерация последовательности x_z для z_tilda
//     std::vector<int8_t> x_z(31, 0);
//     x_z[4] = 1;  // начальные значения: x_z(1:5) = [0 0 0 0 1]
//     for (int i = 0; i < 26; ++i) {
//         x_z[i + 5] = (x_z[i + 4] + x_z[i + 2] + x_z[i + 1] + x_z[i]) % 2;
//     }

//     // Шаг 6: Генерация последовательностей s_tilda, c_tilda и z_tilda
//     std::vector<int8_t> s_tilda(31, 0);
//     std::vector<int8_t> c_tilda(31, 0);
//     std::vector<int8_t> z_tilda(31, 0);
//     for (int i = 0; i < 31; ++i) {
//         s_tilda[i] = 1 - 2 * x_s[i];
//         c_tilda[i] = 1 - 2 * x_c[i];
//         z_tilda[i] = 1 - 2 * x_z[i];
//     }

//     // Шаг 7: Генерация последовательности s0_m0_even
//     std::vector<int8_t> s0_m0_even(31, 0);
//     for (int n = 0; n < 31; ++n) {
//         s0_m0_even[n] = s_tilda[(n + m0) % 31];
//     }

//     // Шаг 8: Генерация последовательности c0_even
//     std::vector<int8_t> c0_even(31, 0);
//     for (int n = 0; n < 31; ++n) {
//         c0_even[n] = c_tilda[(n + N_ID_cell) % 31];
//     }

//     // Шаг 9: Вычисление d_even_sub0
//     std::vector<int8_t> d_even_sub0(31, 0);
//     for (int n = 0; n < 31; ++n) {
//         d_even_sub0[n] = s0_m0_even[n] * c0_even[n];
//     }

//     // Шаг 10: Генерация последовательности s1_m1_odd
//     std::vector<int8_t> s1_m1_odd(31, 0);
//     for (int n = 0; n < 31; ++n) {
//         s1_m1_odd[n] = s_tilda[(n + m1) % 31];
//     }

//     // Шаг 11: Генерация последовательности c1_odd
//     std::vector<int8_t> c1_odd(31, 0);
//     for (int n = 0; n < 31; ++n) {
//         c1_odd[n] = c_tilda[(n + N_ID_cell + 3) % 31];
//     }

//     // Шаг 12: Генерация последовательности z1_m0_odd
//     std::vector<int8_t> z1_m0_odd(31, 0);
//     for (int n = 0; n < 31; ++n) {
//         z1_m0_odd[n] = z_tilda[(n + (m0 % 8)) % 31];
//     }

//     // Шаг 13: Вычисление d_odd_sub0
//     std::vector<int8_t> d_odd_sub0(31, 0);
//     for (int n = 0; n < 31; ++n) {
//         d_odd_sub0[n] = s1_m1_odd[n] * c1_odd[n] * z1_m0_odd[n];
//     }

//     // Шаг 14: Объединение d_even_sub0 и d_odd_sub0 в d_sub0
//     std::vector<int8_t> d_sub0(62, 0);
//     for (int n = 0; n < 31; ++n) {
//         d_sub0[2 * n] = d_even_sub0[n];
//         d_sub0[2 * n + 1] = d_odd_sub0[n];
//     }

//     return d_sub0;
// }

int main(){
    int cell_id = 0;  // Пример ID соты
    auto sss = generate_sss(cell_id);

    // // Выводим сгенерированную последовательность SSS
    // for (int i = 0; i < sss.size(); ++i) {
    //     std::cout << sss[i] << " ";
    // }
    // std::cout << std::endl;
    // for (int i = 0; i < 168; ++i){
    //         
    // }
    //auto sss = generate_sss(33);

    // for (int i = 0; i < 30; ++i){
    //         std::cout << x(i) << " ";
    // }
    

    for(auto s : sss){
        std::cout << s << " ";
    }
    std::cout <<  std::endl;



    return 0;
}
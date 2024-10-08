#ifndef OFDM_MOD_H
#define OFDM_MOD_H

#include <vector>
#include <complex>
#include "../base_define.h"

class OFDM_mod {
public:
    using cd = std::complex<double>;

    // Конструктор класса
    OFDM_mod() {
        N_active_subcarriers = N_FFT - G_SUBCAR;// - N_PILOTS; 
        generateIndices();  // Генерируем индексы данных и пилотов при инициализации
    }

    std::vector<int> data_indices;
    std::vector<int> pilot_indices;

    std::vector<cd> modulate(const std::vector<std::vector<cd>> &input_matrix);
    std::vector<cd> ZadoffChu(int u = 253);
    std::vector<cd> mapPSS();

private:
    int N_active_subcarriers;

    std::vector<cd> mapToSubcarriers(const std::vector<cd> &input);
    void generateIndices();
};

#endif // OFDM_MOD_H

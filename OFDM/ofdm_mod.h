﻿#ifndef OFDM_MOD_H
#define OFDM_MOD_H

#include <vector>
#include <complex>
#include <cmath>
#include <algorithm>
#include "../config.h"

class OFDM_mod {
public:
    using cd = std::complex<double>;
    OFDM_mod();

    std::vector<int> data_indices;
    std::vector<int> pilot_indices;

    std::vector<cd> modulate(const std::vector<std::vector<cd>> &input_matrix);

    std::vector<cd> mapPSS(int u = 25);
    std::vector<cd> ZadoffChu(int u = 25);

    std::vector<cd> mapSSS(int N_ID_cell);
    std::vector<cd> generate_sss(int N_ID_cell);

private:
    int N_active_subcarriers;
    void generateIndices();

    std::vector<cd> mapToSubcarriers(const std::vector<cd> &input);
};

#endif // OFDM_MOD_H

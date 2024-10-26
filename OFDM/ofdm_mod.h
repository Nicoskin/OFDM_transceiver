#ifndef OFDM_MOD_H
#define OFDM_MOD_H

#include <vector>
#include <complex>
#include <cmath>
#include <algorithm>
#include "../config.h"
#include "sequence.h"

using cd = std::complex<double>;

class OFDM_mod {
public:
    OFDM_mod();

    std::vector<int> data_indices;
    std::vector<int> data_indices_noPilots;
    std::vector<int> data_indices_shifted;
    std::vector<int> pilot_indices;
    std::vector<int> pilot_indices_shifted;

    std::vector<cd> modulate(const std::vector<std::vector<cd>> &input_matrix);

    std::vector<cd> mapPSS(int u = 0);

    std::vector<cd> mapSSS(int N_ID_cell);

private:
    int N_active_subcarriers;
    void generateIndices();
    int N_rb = (N_FFT - G_SUBCAR - 1) / 12;
    std::vector<std::vector<std::vector<cd>>> refs{20, std::vector<std::vector<cd>>(7, std::vector<cd>(N_rb * 2, cd(0, 0)))};
    //std::vector<std::vector<std::vector<cd>>> refs;

    std::vector<cd> mapData(const std::vector<cd> &input);
    std::vector<cd> mapPilots(std::vector<cd> &input, uint16_t num_slot, uint16_t num_symbol);
};

 #endif // OFDM_MOD_H

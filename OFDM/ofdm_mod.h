#ifndef OFDM_MOD_H
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
    std::vector<int> data_indices_noPilots;
    std::vector<int> data_indices_shifted;
    std::vector<int> pilot_indices;
    std::vector<int> pilot_indices_shifted;

    std::vector<cd> modulate(const std::vector<std::vector<cd>> &input_matrix);

    std::vector<cd> mapPSS(int u = 25);
    std::vector<cd> ZadoffChu(int u = 25);

    std::vector<cd> mapSSS(int N_ID_cell);
    std::vector<cd> generate_sss(int N_ID_cell);

private:
    int N_active_subcarriers;
    void generateIndices();

    std::vector<cd> mapData(const std::vector<cd> &input);
    std::vector<cd> mapPilots(const std::vector<cd> &input, uint16_t num_slot, uint16_t num_symbol);
};

#endif // OFDM_MOD_H

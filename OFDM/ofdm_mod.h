#ifndef OFDM_MOD_H
#define OFDM_MOD_H

#include <vector>
#include <complex>
#include <cmath>
#include <algorithm>
#include <cstdint>
#include "../config.h"
#include "sequence.h"

using cd = std::complex<double>;

// Structure to hold all indices and references
struct OFDM_Data {
    std::vector<int> data_indices;
    std::vector<int> data_indices_noPilots;
    std::vector<int> data_indices_shifted;
    std::vector<int> pilot_indices;
    std::vector<int> pilot_indices_shifted;

    std::vector<std::vector<std::vector<cd>>> refs;
    int N_rb = ((N_FFT - G_SUBCAR - 1) / 12);
    int N_active_subcarriers = (N_FFT - G_SUBCAR - N_PILOTS - 1);

    OFDM_Data(bool amp_pilots_high = false) {
        refs.resize(20, std::vector<std::vector<cd>>(7, std::vector<cd>(N_rb * 2, cd(0, 0))));
        generateIndices();
        gen_pilots_siq(refs, N_CELL_ID, amp_pilots_high);
    }
    
private:
    void generateIndices();
    
};


class OFDM_mod {
public:
    OFDM_mod(bool amplitude_pilots_high = true);

    std::vector<cd> modulate(const std::vector<std::vector<cd>> &input_matrix);
    std::vector<cd> mapPSS(int u = N_CELL_ID % 3);
    std::vector<cd> mapSSS(int N_ID_cell, int subframe);

    const OFDM_Data& getData() const { return data; }

private:
    int CP_len;
    bool amplitude_pilots_high;
    OFDM_Data data = OFDM_Data(amplitude_pilots_high); 

    std::vector<cd> mapData(const std::vector<cd> &input);
    std::vector<cd> mapPilots(std::vector<cd> &input, uint16_t num_slot, uint16_t num_symbol);
    std::vector<cd> addCyclicPrefix(const std::vector<cd>& time_domain_symbol, int k);
};

#endif // OFDM_MOD_H

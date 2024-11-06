#ifndef OFDM_DEMOD_H
#define OFDM_DEMOD_H

#include <vector>
#include <complex>
#include <cmath>
#include <algorithm>
#include <numeric>

#include "../config.h"

using cd = std::complex<double>;

class OFDM_demod {
public:
    OFDM_demod();

    int CP_len;

    std::vector<cd> demodulate(const std::vector<cd>& signal);

    std::vector<cd> convolve_fft(const std::vector<cd>& vec1, const std::vector<cd>& vec2);

    std::vector<double> correlation(const std::vector<std::complex<double>>& y1, const std::vector<std::complex<double>>& y2);

    std::vector<int> find_indexs_pss(const std::vector<double> corr, float threshold = 0.97);
    std::vector<int> find_max_cp(const std::vector<double>& corr_cp);

    std::vector<cd> extract_slots(const std::vector<cd>& signal, const std::vector<int>& indices, int n_slot);
    std::vector<cd> extract_symb (const std::vector<cd>& signal, const std::vector<int>& indices, int n_symb);

    std::vector<double> corr_cp(const std::vector<cd>& slot_signal);
    std::vector<cd> interpolated_H(const std::vector<cd>& signal, int n_slot, int n_symb);



private:
    std::vector<double> corr_cp_normal(const std::vector<cd>& slot_signal);
    std::vector<double> corr_cp_extended(const std::vector<cd>& slot_signal);

    std::vector<int> find_max_cp_normal(const std::vector<double>& corr_cp);
    std::vector<int> find_max_cp_extended(const std::vector<double>& corr_cp);
    
};


#endif // OFDM_MOD_H

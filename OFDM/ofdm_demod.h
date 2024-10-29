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

    std::vector<cd> convolve(const std::vector<cd>& vec1, const std::vector<cd>& vec2);

    cd correlateStatic(const std::vector<cd>& vec1, const std::vector<cd>& vec2, bool norm = false);
    std::vector<cd> correlateShifted(const std::vector<cd>& vec1, const std::vector<cd>& vec2, bool norm = false);

    std::vector<double> correlate(const std::vector<cd>& vec1, const std::vector<cd>& vec2, bool norm = true);

    int maxIndex(const std::vector<cd>& vec);
    std::vector<cd> CP_CorrIndexs(const std::vector<cd>& vec);


private:
    
};

std::vector<double> correlation(const std::vector<std::complex<double>>& y1, const std::vector<std::complex<double>>& y2);
std::vector<int> find_indexs_pss(std::vector<double> corr, float threshold = 0.97);
std::vector<cd> extract_slots(const std::vector<cd>& signal, const std::vector<int>& indices, int slot_number);
std::vector<double> corr_cp(const std::vector<cd>& slot_signal);
std::vector<int> find_max_cp(const std::vector<double>& corr_cp);

#endif // OFDM_MOD_H

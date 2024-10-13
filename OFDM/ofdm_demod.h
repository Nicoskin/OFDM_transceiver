#ifndef OFDM_DEMOD_H
#define OFDM_DEMOD_H

#include <vector>
#include <complex>
#include "../base_define.h"

class OFDM_demod {
public:
    using cd = std::complex<double>;

    std::vector<cd> convolve(const std::vector<cd>& vec1, const std::vector<cd>& vec2);

    cd correlateStatic(const std::vector<cd>& vec1, const std::vector<cd>& vec2, bool norm = false);
    std::vector<cd> correlateShifted(const std::vector<cd>& vec1, const std::vector<cd>& vec2, bool norm = false);


private:
    int N_active_subcarriers;
    
};

#endif // OFDM_MOD_H

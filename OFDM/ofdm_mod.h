#ifndef OFDM_MOD_H
#define OFDM_MOD_H

#include <vector>
#include <complex>
#include "../base_define.h"

class OFDM {
public:
    using cd = std::complex<double>;

    std::vector<cd> process(const std::vector<std::vector<cd>> &input_matrix);
    std::vector<cd> ZadoffChu(bool PSS = false, int u = 25, int N = 63);
    std::vector<cd> mapPSS(const std::vector<cd> &pss);

private:
    int N_active_subcarriers;
    int N_active_subcar_for_pss;

    std::vector<cd> mapToSubcarriers(const std::vector<cd> &input);
    
};

#endif // OFDM_MOD_H

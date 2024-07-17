#ifndef OFDM_MOD_H
#define OFDM_MOD_H

#include <vector>
#include <complex>

class OFDM {
public:
    using cd = std::complex<double>;

    OFDM(int N_FFT = 128, int G_subcar = 55, int N_PILOTS = 6, int CP_LEN = 16);
    std::vector<cd> process(const std::vector<std::vector<cd>> &input_matrix);
    std::vector<cd> ZadoffChu(bool PSS = false, int u = 25, int N = 63);
    std::vector<cd> mapPSS(const std::vector<cd> &pss);

private:
    int N_FFT;
    int G_subcar;
    int N_PILOTS;
    int CP_LEN;
    int N_active_subcarriers;

    std::vector<cd> mapToSubcarriers(const std::vector<cd> &input);
    
};

#endif // OFDM_MOD_H

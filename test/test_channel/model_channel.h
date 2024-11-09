#include <vector>
#include <complex>
#include <random>


using cd = std::complex<double>;

std::vector<cd> add_noise(const std::vector<cd>& signal, double SNR_dB, unsigned int seed);

std::vector<cd> add_CFO(std::vector<cd>& signal, double CFO = 1500, uint32_t F_srate = 1920000);

std::vector<std::complex<double>> pad_zeros(const std::vector<std::complex<double>>& signal,  size_t num_zeros_front, size_t num_zeros_back);

#ifndef MODULATION_H
#define MODULATION_H

#include <vector>
#include <complex>
#include <memory>
#include <cmath>

class Modulation {
public:
    Modulation(size_t length);
    
    void setSymbols(const std::vector<std::complex<double>>& symbolMap, const std::vector<int>& bits, size_t symbolSize, double normalizationFactor);

    std::vector<std::complex<double>> symbols;

};

std::vector<std::vector<std::complex<double>>> modulate(const std::vector<std::vector<uint8_t>>& bits, int modulation);

#endif // MODULATION_H

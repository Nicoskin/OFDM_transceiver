#ifndef MODULATION_H
#define MODULATION_H

#include <vector>
#include <complex>
#include <memory>

enum ModulationType {
    BPSK,
    QPSK,
    QAM16,
    QAM64
};

class Modulation {
public:
    Modulation(ModulationType modulationType, size_t length);
    
    void setSymbols(const std::vector<std::complex<double>>& symbolMap, const std::vector<int>& bits, size_t symbolSize, double normalizationFactor);

    std::vector<std::complex<double>> symbols;

private:
    ModulationType modulation;
};

std::vector<std::vector<std::complex<double>>> modulate(const std::vector<std::vector<uint8_t>>& bits, ModulationType modulation);

#endif // MODULATION_H

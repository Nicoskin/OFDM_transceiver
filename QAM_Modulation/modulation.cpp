#include "modulation.h"
#include <iostream>
#include <cmath>
#include <complex>
#include <memory>
#include <vector>

namespace {
    using cd = std::complex<double>;
    using namespace std::complex_literals;  
}

static const std::vector<cd> BPSK_MAP = {1.0 + 0.0i, -1.0 + 0.0i};
//static const std::vector<cd> BPSK_MAP = {1.0 + 1.0i, -1.0 - 1.0i};         // +1+1 -1-1
static const std::vector<cd> QPSK_MAP = {1.0 + 1.0i, 1.0 - 1.0i, -1.0 + 1.0i, -1.0 - 1.0i};
static const std::vector<cd> QAM16_MAP = {
    1.0 + 1.0i, 1.0 + 3.0i, 3.0 + 1.0i, 3.0 + 3.0i,
    1.0 - 1.0i, 1.0 - 3.0i, 3.0 - 1.0i, 3.0 - 3.0i,
    -1.0 + 1.0i, -1.0 + 3.0i, -3.0 + 1.0i, -3.0 + 3.0i,
    -1.0 - 1.0i, -1.0 - 3.0i, -3.0 - 1.0i, -3.0 - 3.0i
};
static const std::vector<cd> QAM64_MAP = {
    1.0 + 1.0i, 1.0 + 3.0i, 1.0 + 5.0i, 1.0 + 7.0i,
    3.0 + 1.0i, 3.0 + 3.0i, 3.0 + 5.0i, 3.0 + 7.0i,
    5.0 + 1.0i, 5.0 + 3.0i, 5.0 + 5.0i, 5.0 + 7.0i,
    7.0 + 1.0i, 7.0 + 3.0i, 7.0 + 5.0i, 7.0 + 7.0i,
    1.0 - 1.0i, 1.0 - 3.0i, 1.0 - 5.0i, 1.0 - 7.0i,
    3.0 - 1.0i, 3.0 - 3.0i, 3.0 - 5.0i, 3.0 - 7.0i,
    5.0 - 1.0i, 5.0 - 3.0i, 5.0 - 5.0i, 5.0 - 7.0i,
    7.0 - 1.0i, 7.0 - 3.0i, 7.0 - 5.0i, 7.0 - 7.0i,
    -1.0 + 1.0i, -1.0 + 3.0i, -1.0 + 5.0i, -1.0 + 7.0i,
    -3.0 + 1.0i, -3.0 + 3.0i, -3.0 + 5.0i, -3.0 + 7.0i,
    -5.0 + 1.0i, -5.0 + 3.0i, -5.0 + 5.0i, -5.0 + 7.0i,
    -7.0 + 1.0i, -7.0 + 3.0i, -7.0 + 5.0i, -7.0 + 7.0i,
    -1.0 - 1.0i, -1.0 - 3.0i, -1.0 - 5.0i, -1.0 - 7.0i,
    -3.0 - 1.0i, -3.0 - 3.0i, -3.0 - 5.0i, -3.0 - 7.0i,
    -5.0 - 1.0i, -5.0 - 3.0i, -5.0 - 5.0i, -5.0 - 7.0i,
    -7.0 - 1.0i, -7.0 - 3.0i, -7.0 - 5.0i, -7.0 - 7.0i
};

Modulation::Modulation(ModulationType modulationType, size_t length)
    : modulation(modulationType), symbols(length) {}

void Modulation::setSymbols(const std::vector<cd>& symbolMap, const std::vector<int>& bits, size_t symbolSize, double normalizationFactor) {
    for (size_t i = 0; i < symbols.size(); ++i) {
        int index = 0;
        for (size_t j = 0; j < symbolSize; ++j) {
            index = (index << 1) | bits[i * symbolSize + j];
        }
        symbols[i] = symbolMap[index] / std::sqrt(normalizationFactor);
    }
}

std::vector<std::vector<cd>> modulate(const std::vector<std::vector<uint8_t>>& bits, ModulationType modulation) {
    size_t symbolSize = 0;
    double normalizationFactor = 1.0;
    const std::vector<cd>* symbolMap;

    switch (modulation) {
        case BPSK:
            symbolSize = 1;
            normalizationFactor = 1.0;
            symbolMap = &BPSK_MAP;
            break;

        case QPSK:
            symbolSize = 2;
            normalizationFactor = 2.0;
            symbolMap = &QPSK_MAP;
            break;

        case QAM16:
            symbolSize = 4;
            normalizationFactor = 10.0;
            symbolMap = &QAM16_MAP;
            break;

        case QAM64:
            symbolSize = 6;
            normalizationFactor = 42.0;
            symbolMap = &QAM64_MAP;
            break;

        default:
            std::cerr << "Unsupported modulation type" << std::endl;
            return {};
    }

    std::vector<std::vector<cd>> result;

    for (const auto& bitVec : bits) {
        std::vector<int> intBits(bitVec.begin(), bitVec.end());
        
        while (intBits.size() % symbolSize != 0) {
            intBits.push_back(0);
        }

        size_t numSymbols = intBits.size() / symbolSize;
        Modulation mod(modulation, numSymbols);
        mod.setSymbols(*symbolMap, intBits, symbolSize, normalizationFactor);

        result.push_back(mod.symbols);
    }

    return result;
}

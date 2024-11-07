#include "qam_mod.h"
#include <iostream>


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
    3.0 + 3.0i, 3.0 + 1.0i, 1.0 + 3.0i, 1.0 + 1.0i, // 000000 - 000011
    3.0 + 5.0i, 3.0 + 7.0i, 1.0 + 5.0i, 1.0 + 7.0i, // 000100 - 000111
    5.0 + 3.0i, 5.0 + 1.0i, 7.0 + 3.0i, 7.0 + 1.0i, // 001000 - 001011
    5.0 + 5.0i, 5.0 + 7.0i, 7.0 + 5.0i, 7.0 + 7.0i, // 001100 - 001111
    3.0 - 3.0i, 3.0 - 1.0i, 1.0 - 3.0i, 1.0 - 1.0i, // 010000 - 010011
    3.0 - 5.0i, 3.0 - 7.0i, 1.0 - 5.0i, 1.0 - 7.0i, // 010100 - 010111
    5.0 - 3.0i, 5.0 - 1.0i, 7.0 - 3.0i, 7.0 - 1.0i, // 011000 - 011011
    5.0 - 5.0i, 5.0 - 7.0i, 7.0 - 5.0i, 7.0 - 7.0i, // 011100 - 011111
   -3.0 + 3.0i, -3.0 + 1.0i, -1.0 + 3.0i, -1.0 + 1.0i, // 100000 - 100011
   -3.0 + 5.0i, -3.0 + 7.0i, -1.0 + 5.0i, -1.0 + 7.0i, // 100100 - 100111
   -5.0 + 3.0i, -5.0 + 1.0i, -7.0 + 3.0i, -7.0 + 1.0i, // 101000 - 101011
   -5.0 + 5.0i, -5.0 + 7.0i, -7.0 + 5.0i, -7.0 + 7.0i, // 101100 - 101111
   -3.0 - 3.0i, -3.0 - 1.0i, -1.0 - 3.0i, -1.0 - 1.0i, // 110000 - 110011
   -3.0 - 5.0i, -3.0 - 7.0i, -1.0 - 5.0i, -1.0 - 7.0i, // 110100 - 110111
   -5.0 - 3.0i, -5.0 - 1.0i, -7.0 - 3.0i, -7.0 - 1.0i, // 111000 - 111011
   -5.0 - 5.0i, -5.0 - 7.0i, -7.0 - 5.0i, -7.0 - 7.0i  // 111100 - 111111
};
// Карта символов для быстрого доступа
const std::unordered_map<ModulationType, std::vector<cd>> QAM_mod::symbolMaps = {
    {BPSK, BPSK_MAP},
    {QPSK, QPSK_MAP},
    {QAM16, QAM16_MAP},
    {QAM64, QAM64_MAP},
};

// Параметры модуляции: количество бит на символ и нормализующий коэффициент
const std::unordered_map<ModulationType, std::pair<size_t, double>> QAM_mod::modulationParameters = {
    {BPSK, {1, 1.0}},
    {QPSK, {2, 2.0}},
    {QAM16, {4, 10.0}},
    {QAM64, {6, 42.0}},
};


// Установить символы на основе битов
void QAM_mod::setSymbols(const std::vector<cd>& symbolMap, const std::vector<int>& bits, size_t symbolSize, double normalizationFactor) {
    size_t numSymbols = bits.size() / symbolSize;
    symbols.resize(numSymbols); // Изменяем размер вектора символов

    for (size_t i = 0; i < numSymbols; ++i) {
        int index = 0;
        for (size_t j = 0; j < symbolSize; ++j) {
            index = (index << 1) | bits[i * symbolSize + j];
        }
        symbols[i] = symbolMap[index] / std::sqrt(normalizationFactor);
    }
}

// Модуляция на основе типа модуляции
std::vector<std::vector<cd>> QAM_mod::modulate(const std::vector<std::vector<uint8_t>>& bits, ModulationType modulationType) {
    // If modulationType is not provided, use IQ_MODULATION to set the default
    if (modulationType == ModulationType::None) {
        switch (IQ_MODULATION) {
            case 1:
                modulationType = ModulationType::BPSK;
                break;
            case 2:
                modulationType = ModulationType::QPSK;
                break;
            case 4:
                modulationType = ModulationType::QAM16;
                break;
            case 6:
                modulationType = ModulationType::QAM64;
                break;
            default:
                std::cerr << "Unsupported IQ_MODULATION value" << std::endl;
                return {};
        }
    }

    // Ensure the modulationType is valid
    if (modulationParameters.find(modulationType) == modulationParameters.end()) {
        std::cerr << "Unsupported modulation type" << std::endl;
        return {};
    }

    size_t symbolSize = modulationParameters.at(modulationType).first;
    double normalizationFactor = modulationParameters.at(modulationType).second;
    const std::vector<cd>& symbolMap = symbolMaps.at(modulationType);

    std::vector<std::vector<cd>> result;

    for (const auto& bitVec : bits) {
        std::vector<int> intBits(bitVec.begin(), bitVec.end());

        // Pad with zeros if not a multiple of symbol size
        while (intBits.size() % symbolSize != 0) {
            intBits.push_back(0);
        }

        // Set symbols and modulate
        setSymbols(symbolMap, intBits, symbolSize, normalizationFactor);
        result.push_back(symbols);
    }

    return result;
}
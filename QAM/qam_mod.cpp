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

// Карта символов для быстрого доступа
const std::unordered_map<ModulationType, std::vector<cd>> QAM_mod::symbolMaps = {
    {BPSK, BPSK_MAP},
    {QPSK, QPSK_MAP},
    {QAM16, QAM16_MAP},
    {QAM64, QAM64_MAP}
};

// Параметры модуляции: количество бит на символ и нормализующий коэффициент
const std::unordered_map<ModulationType, std::pair<size_t, double>> QAM_mod::modulationParameters = {
    {BPSK, {1, 1.0}},
    {QPSK, {2, 2.0}},
    {QAM16, {4, 10.0}},
    {QAM64, {6, 42.0}}
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

        // Дополняем нулями, если не кратно размеру символа
        while (intBits.size() % symbolSize != 0) {
            intBits.push_back(0);
        }

        // Устанавливаем символы и модулируем
        setSymbols(symbolMap, intBits, symbolSize, normalizationFactor);
        result.push_back(symbols);
    }

    return result;
}
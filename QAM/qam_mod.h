#ifndef QAM_MOD_H
#define QAM_MOD_H

#include <vector>
#include <complex>
#include <cmath>
#include <unordered_map>

using cd = std::complex<double>;

// Перечисление типов модуляции
enum ModulationType {
    BPSK,
    QPSK,
    QAM16,
    QAM64
};

class QAM_mod {
public:
    // Модуляция на основе битовой матрицы и типа модуляции
    std::vector<std::vector<cd>> modulate(const std::vector<std::vector<uint8_t>>& bits, ModulationType modulationType);

private:
    // Установить символы на основе карты символов и битов
    void setSymbols(const std::vector<cd>& symbolMap, const std::vector<int>& bits, size_t symbolSize, double normalizationFactor);

    std::vector<cd> symbols;

    // Карты символов для различных типов модуляции
    static const std::unordered_map<ModulationType, std::vector<cd>> symbolMaps;
    static const std::unordered_map<ModulationType, std::pair<size_t, double>> modulationParameters;
};

#endif // QAM_MOD_H
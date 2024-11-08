#include "qam_demod.h"
#include "qam_mod.h"

namespace {
    using cd = std::complex<double>;
    using namespace std::complex_literals;  
}

QAM_demod::QAM_demod() {
    generateConstellation();
}

void QAM_demod::generateConstellation() {
    switch (IQ_MODULATION) {
        case 1:
            constellation = {1.0 + 0.0i, -1.0 + 0.0i};
            break;
        case 2:
            constellation = {1.0 + 1.0i, 1.0 - 1.0i, -1.0 + 1.0i, -1.0 - 1.0i};
            for (size_t i = 0; i < constellation.size(); ++i) {
                constellation[i] = constellation[i] / std::sqrt(2); 
            }
            break;
        case 4:
            constellation = {
                1.0 + 1.0i, 1.0 + 3.0i, 3.0 + 1.0i, 3.0 + 3.0i,
                1.0 - 1.0i, 1.0 - 3.0i, 3.0 - 1.0i, 3.0 - 3.0i,
                -1.0 + 1.0i, -1.0 + 3.0i, -3.0 + 1.0i, -3.0 + 3.0i,
                -1.0 - 1.0i, -1.0 - 3.0i, -3.0 - 1.0i, -3.0 - 3.0i
            };
            for (size_t i = 0; i < constellation.size(); ++i) {
                constellation[i] = constellation[i] / std::sqrt(10); 
            }
            break;
        case 6:
            constellation = {
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
            for (size_t i = 0; i < constellation.size(); ++i) {
                constellation[i] = constellation[i] / std::sqrt(42); 
            }
            break;
    }
}

std::vector<double> QAM_demod::calculat_softDecision(const std::complex<double>& point) {
    std::vector<double> distances;

    for (const auto& symbol : constellation) {
        double distance = std::pow(std::abs(point - symbol), 2);
        distances.push_back(distance);
    }

    return distances;
}

std::vector<std::vector<double>> QAM_demod::softDecision(const std::vector<cd>& receivedSignal) {
    std::vector<std::vector<double>> softDecisions;

    for (const auto& point : receivedSignal) {
        softDecisions.push_back(calculat_softDecision(point));
    }

    return softDecisions;
}

std::vector<uint8_t> QAM_demod::demodulate(const std::vector<cd>& receivedSignal) {
    std::vector<uint8_t> bits;
    int bitsPerSymbol = IQ_MODULATION;

    auto softD = softDecision(receivedSignal);

    // Преобразуем каждый мягкий символ в биты
    for (const auto& symbolDistances : softD) {
        // Найти индекс минимального расстояния вручную
        double minDistance = symbolDistances[0];
        int index = 0;

        for (size_t i = 1; i < symbolDistances.size(); ++i) {
            if (symbolDistances[i] < minDistance) {
                minDistance = symbolDistances[i];
                index = i;
            }
        }
        // Найти индекс минимального расстояния
        // auto minIt = std::min_element(symbolDistances.begin(), symbolDistances.end());
        // int index = std::distance(symbolDistances.begin(), minIt);

        // Преобразовать индекс в битовую последовательность
        for (int i = bitsPerSymbol - 1; i >= 0; --i) {
            bits.push_back((index >> i) & 1);
        }
    }

    return bits;
}

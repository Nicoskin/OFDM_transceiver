#include "qam_demod.h"
#include "qam_mod.h"

namespace {
    using cd = std::complex<double>;
    using namespace std::complex_literals;  
}

QAMDemodulator::QAMDemodulator(ModulationType modScheme) : modScheme(modScheme) {
    generateConstellation();
}

void QAMDemodulator::generateConstellation() {
    switch (modScheme) {
        case BPSK:
            constellation = {1.0 + 0.0i, -1.0 + 0.0i};
            break;
        case QPSK:
            constellation = {1.0 + 1.0i, 1.0 - 1.0i, -1.0 + 1.0i, -1.0 - 1.0i};
            for (size_t i = 0; i < constellation.size(); ++i) {
                constellation[i] = constellation[i] / std::sqrt(2); 
            }
            break;
        case QAM16:
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
        case QAM64:
            constellation = {
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
            for (size_t i = 0; i < constellation.size(); ++i) {
                constellation[i] = constellation[i] / std::sqrt(42); 
            }
            break;
    }
}

std::vector<double> QAMDemodulator::softDecision(const std::complex<double>& point) {
    std::vector<double> distances;

    for (const auto& symbol : constellation) {
        double distance = std::pow(std::abs(point - symbol), 2);
        distances.push_back(distance);
    }

    return distances;
}

std::vector<std::vector<double>> QAMDemodulator::demodulate(const std::vector<std::complex<double>>& receivedSignal) {
    std::vector<std::vector<double>> softDecisions;

    for (const auto& point : receivedSignal) {
        softDecisions.push_back(softDecision(point));
    }

    return softDecisions;
}

std::vector<int> QAMDemodulator::softDecisionsToBits(const std::vector<std::vector<double>>& softDecisions) {
    std::vector<int> bits;
    int bitsPerSymbol = 0;

    switch (modScheme) {
        case BPSK:
            bitsPerSymbol = 1;
            break;
        case QPSK:
            bitsPerSymbol = 2;
            break;
        case QAM16:
            bitsPerSymbol = 4;
            break;
        case QAM64:
            bitsPerSymbol = 6;
            break;
    }

    // Преобразуем каждый мягкий символ в биты
    for (const auto& symbolDistances : softDecisions) {
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

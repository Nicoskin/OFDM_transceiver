#include "qam_demod.h"

namespace {
    using cd = std::complex<double>;
    using namespace std::complex_literals;  
}

QAMDemodulator::QAMDemodulator(ModulationScheme modScheme) : modScheme(modScheme) {
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

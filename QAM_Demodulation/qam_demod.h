#ifndef QAM_DEMOD_H
#define QAM_DEMOD_H

#include <vector>
#include <complex>
#include <cmath>

#include <bitset>
#include <map>

enum ModulationScheme { BPSK, QPSK, QAM16, QAM64 };

class QAMDemodulator {
public:
    QAMDemodulator(ModulationScheme modScheme);

    std::vector<std::vector<double>> demodulate(const std::vector<std::complex<double>>& receivedSignal);
    std::vector<int> softDecisionsToBits(const std::vector<std::vector<double>>& softDecisions);

private:
    ModulationScheme modScheme;
    std::vector<std::complex<double>> constellation;

    void generateConstellation();
    std::vector<double> softDecision(const std::complex<double>& point);
};

#endif // QAM_DEMOD_H

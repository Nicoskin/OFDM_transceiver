#ifndef QAM_DEMOD_H
#define QAM_DEMOD_H

#include <vector>
#include <complex>
#include <cmath>

enum ModulationScheme { BPSK, QPSK, QAM16, QAM64 };

class QAMDemodulator {
public:
    QAMDemodulator(ModulationScheme modScheme);

    std::vector<std::vector<double>> demodulate(const std::vector<std::complex<double>>& receivedSignal);

private:
    ModulationScheme modScheme;
    std::vector<std::complex<double>> constellation;

    void generateConstellation();
    std::vector<double> softDecision(const std::complex<double>& point);
};

#endif // QAM_DEMOD_H

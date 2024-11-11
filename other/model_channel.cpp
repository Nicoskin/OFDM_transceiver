#include "model_channel.h"

#define M_PI 3.14159265358979323846

using cd = std::complex<double>;

std::vector<cd> add_noise(const std::vector<cd>& signal, double SNR_dB, unsigned int seed) {
    // Вычисляем среднеквадратическое значение амплитуды сигнала
    double As = 0.0;
    for (const auto& s : signal) {
        As += std::norm(s);  // norm дает квадрат модуля комплексного числа
    }
    As = std::sqrt(As / signal.size());  // Среднеквадратическое значение амплитуды сигнала

    // Перевод SNR из dB в линейное значение
    double SNR_linear = std::pow(10.0, SNR_dB / 20.0);

    // Вычисляем среднеквадратическое значение амплитуды шума
    double An = As / SNR_linear;

    // Дисперсия шума (мощность каждой компоненты) с учетом амплитуды шума
    double noise_variance = std::pow(An, 2) / 2.0;

    // Настройка генератора случайных чисел с нормальным распределением и заданным seed
    std::default_random_engine generator;
    if (seed != 0) {
        generator.seed(seed);
    } else {
        generator.seed(std::random_device{}());
    }
    std::normal_distribution<double> distribution(0.0, std::sqrt(noise_variance));

    // Генерируем шум
    std::vector<cd> noise(signal.size());
    for (auto& n : noise) {
        double real_part = distribution(generator);
        double imag_part = distribution(generator);
        n = cd(real_part, imag_part);
    }
    // Добавляем шум к сигналу
    std::vector<cd> noisy_signal(signal.size());
    for (size_t i = 0; i < signal.size(); ++i) {
        noisy_signal[i] = signal[i] + noise[i];
    }

    return noisy_signal;
}

// Carrier Frequency Offset
std::vector<cd> add_CFO(std::vector<cd>& signal, double CFO, uint32_t F_srate) {
    double phase = 0.0;
    std::vector<cd> signal_CFO(signal.size());
    double phase_increment = 2 * M_PI * CFO/F_srate;  // CFO = Сколько кручейни фазы за 1 сек / частота дискретизации (1536/1_920_000) = 0,0008
    
    for (int i = 0; i < signal.size(); ++i) {
        signal_CFO[i] = signal[i] * std::exp(cd(0.0, phase));
        phase += phase_increment;
    }
    return signal_CFO;
}

std::vector<std::complex<double>> pad_zeros(const std::vector<std::complex<double>>& signal,  size_t num_zeros_front, size_t num_zeros_back) {
    // Создаем новый вектор с необходимым размером
    std::vector<std::complex<double>> padded_signal;
    padded_signal.reserve(num_zeros_front + signal.size() + num_zeros_back);

    // Добавляем нули в начало
    padded_signal.insert(padded_signal.end(), num_zeros_front, std::complex<double>(0.0, 0.0));

    // Копируем оригинальный сигнал
    padded_signal.insert(padded_signal.end(), signal.begin(), signal.end());

    // Добавляем нули в конец
    padded_signal.insert(padded_signal.end(), num_zeros_back, std::complex<double>(0.0, 0.0));

    return padded_signal;
}

// some multipath impulse response 
// h = {{1.0, 0.0}, {0.6, 0.1}, {0.4, -0.3}}
std::vector<cd> add_Channel(const std::vector<cd>& signal, const std::vector<cd>& h) {
    std::vector<cd> output(signal.size() + h.size() - 1, cd(0.0, 0.0));

    for (size_t i = 0; i < signal.size(); ++i) {
        for (size_t j = 0; j < h.size(); ++j) {
            output[i + j] += signal[i] * h[j];
        }
    }

    return output;
}
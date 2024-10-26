#include <iostream>
#include <fstream>
#include <vector>
#include <complex>
#include <string>
#include <cmath>

#include "../../QAM/qam_mod.h"
#include "../../QAM/qam_demod.h"
#include "../../Segmenter/segmenter.h"
#include "../../OFDM/ofdm_mod.h"
#include "../../OFDM/ofdm_demod.h"
#include "../../OFDM/sequence.h"
#include "../../File_converter/file_converter.h"

using cd = std::complex<double>;

// g++ test.cpp  ../../File_converter/file_converter.cpp  ../../QAM/qam_mod.cpp ../../QAM/qam_demod.cpp ../../Segmenter/segmenter.cpp ../../OFDM/ofdm_mod.cpp ../../OFDM/ofdm_demod.cpp ../../OFDM/fft/fft.cpp ../../OFDM/sequence.cpp -o test && ./test

std::vector<cd> generate_noise(const std::vector<cd>& signal, double SNR_dB) {
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

    // Настройка генератора случайных чисел с нормальным распределением
    std::default_random_engine generator;
    std::normal_distribution<double> distribution(0.0, std::sqrt(noise_variance));

    // Генерируем шум
    std::vector<std::complex<double>> noise(signal.size());
    for (auto& n : noise) {
        double real_part = distribution(generator);
        double imag_part = distribution(generator);
        n = std::complex<double>(real_part, imag_part);
    }

    return noise;
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

int main() {

    Segmenter segmenter;
    auto bits = generateRandBits(500, 1);
    auto segments = segmenter.segment(bits);
    segments = segmenter.scramble(segments);

    QAM_mod qam_mod;
    auto qpsk_mod = qam_mod.modulate(segments, QPSK);

    OFDM_mod ofdm_mod;
    auto ofdm_data = ofdm_mod.modulate(qpsk_mod);

    double SNR_dB = 10.0;
    auto signal = pad_zeros(ofdm_data, 1000, 1000);
    auto noise = generate_noise(signal, SNR_dB);
    std::vector<cd> noisy_signal;
    for (int i = 0; i < signal.size(); ++i) {
        noisy_signal.push_back(signal[i] + noise[i]);
    }




    return 0;
}
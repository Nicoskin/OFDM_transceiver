﻿#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include "../../QAM_Modulation/modulation.h"
#include "../qam_demod.h"

// g++ test.cpp ../../QAM_Modulation/modulation.cpp ../qam_demod.cpp -o test && ./test

namespace {
    using cd = std::complex<double>;
    using namespace std::complex_literals;  
}

// Функция для преобразования SNR в стандартное отклонение шума
double snr_to_stddev(double snr) {
    return std::sqrt(1.0 / (2.0 * std::pow(10.0, snr / 10.0)));
}

std::vector<cd> add_noise_to_qpsk(const std::vector<std::vector<cd>>& qpsk_data, double snr, bool fixed_seed = false) {
    std::vector<cd> noisy_signal;
    
    double stddev = snr_to_stddev(snr);

    // Настраиваем генератор случайных чисел для нормального распределения
    std::random_device rd;
    std::mt19937 gen(fixed_seed ? 42 : rd());  // Если fixed_seed == true, используем фиксированный сид
    std::normal_distribution<> d(0.0, stddev);

    // Проходим по каждому элементу qpsk_data и добавляем к нему шум
    for (const auto& row : qpsk_data) {
        for (const auto& symbol : row) {
            double noise_real = d(gen); // Шум для действительной части
            double noise_imag = d(gen); // Шум для мнимой части
            cd noisy_symbol = symbol + cd(noise_real, noise_imag); // Добавляем шум к символу
            noisy_signal.push_back(noisy_symbol);
        }
    }

    return noisy_signal;
}

int main() {
    std::vector<std::vector<uint8_t>> bits = {{1, 1, 0, 1, 1, 1, 1, 0}}; 

    auto qpsk_mod = modulate(bits, 2);

    std::cout << "Modulated Symbols:\n";
    for (const auto& symbol_vec : qpsk_mod) {
        for (const auto& symbol : symbol_vec) {
            std::cout << "(" << symbol.real() << ", " << symbol.imag()  << "),  \n"; 
        }
        std::cout << std::endl;
    }


    // Параметры SNR и флаг для фиксации сида
    double snr = 10.0; // Пример SNR в dB
    bool fixed_seed = true; // Установить в true для фиксированного сида

    // Добавляем шум к данным QPSK
    std::vector<cd> noisy_signal = add_noise_to_qpsk(qpsk_mod, snr, fixed_seed);

    // Выводим результат
    std::cout << "Noisy Signal:\n";
    for (const auto& symbol : noisy_signal) {
        std::cout << symbol << "\n";
    }
    std::cout << std::endl;

    // Демодуляция сигнала
    QAMDemodulator QAMDemod(QPSK);
    auto demod = QAMDemod.demodulate(noisy_signal);

    std::cout << "Demodulated Metrics:\n";
    for (const auto& symbol : demod) {
        for (const auto& metric : symbol) {
            std::cout << metric << ", ";
        }
        std::cout << "\n";
    }

    return 0;
}
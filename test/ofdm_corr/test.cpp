#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include "../../QAM/qam_mod.h"
#include "../../QAM/qam_demod.h"
#include "../../Segmenter/segmenter.h"
#include "../../OFDM/ofdm_mod.h"
#include "../../OFDM/ofdm_demod.h"
#include "../../OFDM/sequence.h"
#include "../../File_converter/file_converter.h"

// g++ test.cpp  ../../File_converter/file_converter.cpp  ../../QAM/qam_mod.cpp ../../QAM/qam_demod.cpp ../../Segmenter/segmenter.cpp ../../OFDM/ofdm_mod.cpp ../../OFDM/ofdm_demod.cpp ../../OFDM/fft/fft.cpp -o test && ./test

namespace {
    using cd = std::complex<double>;
    using namespace std::complex_literals;  
}

double calculate_signal_power(const std::vector<cd>& data) {
    double power = 0.0;
    for (const auto& sym : data) {
        power += std::abs(sym);  // std::norm() возвращает квадрат модуля комплексного числа
    }
    return power / data.size();  // Средняя мощность сигнала
}

std::vector<cd> add_noise(const std::vector<cd>& data, double snr, bool fixed_seed = false) {
    std::vector<cd> noisy_signal;
    
    // Вычисляем мощность сигнала
    double signal_power = calculate_signal_power(data);

    double stddev = signal_power / snr;

    // Настраиваем генератор случайных чисел для нормального распределения
    std::random_device rd;
    std::mt19937 gen(fixed_seed ? 42 : rd());  // Если fixed_seed == true, используем фиксированный сид
    std::normal_distribution<> d(0.0, stddev);

    for (const auto& sym : data) {
        // Генерируем шум для действительной и мнимой частей
        double noise_real = d(gen);
        double noise_imag = d(gen);
        // Добавляем шум к символу
        noisy_signal.push_back(sym + cd(noise_real, noise_imag));
    }

    return noisy_signal;
}

int main() {
    // Входные биты
    // std::vector<uint8_t> bits = {
    // 0,1,1,0,1,0,0,1, 1,0,0,1,0,1,1,0, 1,0,0,1,0,1,1,0,
    // 1,0,0,1,0,1,1,0, 1,0,0,1,0,1,1,0,
    // 0,1,1,0,1,0,0,1, 1,1,1,1,1,1,1,1,
    // 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,}; 

    auto bits = generateRandBits(500);

    // Сегментер
    Segmenter segmenter;
    auto segments = segmenter.segment(bits);
    //std::cout << segments[0].size() << std::endl;
    segments = segmenter.scramble(segments);

    QAM_mod qam_mod; 
    auto qpsk_mod = qam_mod.modulate(segments, QPSK);

    // std::cout << "Modulated Symbols:\n";
    // for (const auto& symbol_vec : qpsk_mod) {
    //     for (const auto& symbol : symbol_vec) {
    //         std::cout << "(" << symbol.real() << ", " << symbol.imag()  << "),  \n"; 
    //     }
    //     std::cout << std::endl;
    // }

    OFDM_mod ofdm_mod;
    auto ofdm_data = ofdm_mod.modulate(qpsk_mod);

    // std::cout << "OFDM samples:\n";
    // for (const auto& sym : ofdm_data) {
    //     std::cout << sym << ",\n";
    // }
    // std::cout << std::endl;

    // Параметры SNR и флаг для фиксации сида
    double snr = 10.0; // Пример SNR в dB
    bool fixed_seed = false; // Установить в true для фиксированного сида

    // Добавляем шум к данным QPSK
    std::vector<cd> noisy_signal = add_noise(ofdm_data, snr, fixed_seed);

    // // Выводим результат
    // std::cout << "Noisy Signal:\n";
    // for (const auto& symbol : noisy_signal) {
    //     std::cout << symbol << ",\n";
    // }
    // std::cout << std::endl;

    auto pss = ofdm_mod.mapPSS();

    // std::cout << "pss:\n";
    // for (const auto& symbol : pss) {
    //     std::cout << symbol << ",\n";
    // }
    // std::cout << std::endl;


    OFDM_demod ofdm_demod;
    auto corr = ofdm_demod.correlate(ofdm_data, pss);
    //auto corr = ofdm_demod.correlateStatic(pss, pss, true);
    // auto corr = ofdm_demod.convolve(pss, pss);
    //std::cout << corr << std::endl;
    // Выводим результат
    // std::cout << "Corr Signal:\n";
    // for (const auto& symbol : corr) {
    //     std::cout << symbol << ",\n";
    // }
    std::cout << std::endl;
/*
    std::vector<int> out_bits = QAMDemod.softDecisionsToBits(softDecisions);
    std::cout << "\nDemodulated Bits:\n";
    for (const auto& bit : out_bits) {
        std::cout << bit << " ";
    }
    std::cout << std::endl;
*/

    return 0;
}

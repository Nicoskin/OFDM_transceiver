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

std::vector<cd> generate_noise(const std::vector<cd>& signal, double SNR_dB, unsigned int seed) {
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
    std::default_random_engine generator(seed);
    std::normal_distribution<double> distribution(0.0, std::sqrt(noise_variance));

    // Генерируем шум
    std::vector<cd> noise(signal.size());
    for (auto& n : noise) {
        double real_part = distribution(generator);
        double imag_part = distribution(generator);
        n = cd(real_part, imag_part);
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

void saveCorr(const std::vector<double>& corr, const std::string& filename) {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cerr << "Не удалось открыть файл для записи." << std::endl;
        return;
    }

    for (const auto& value : corr) {
        outFile << value << "\n";
    }
}

int main() {

    Segmenter segmenter;
    auto bits = generateRandBits(500, 2);
    auto segments = segmenter.segment(bits);
    segments = segmenter.scramble(segments);

    QAM_mod qam_mod;
    auto qpsk_mod = qam_mod.modulate(segments, QPSK);

    OFDM_mod ofdm_mod;
    auto ofdm_data = ofdm_mod.modulate(qpsk_mod);

    double SNR_dB = 10.0;
    auto signal = pad_zeros(ofdm_data, 1000, 1000);
    auto noise = generate_noise(signal, SNR_dB, 1);
    std::vector<cd> noise_signal;
    for (int i = 0; i < signal.size(); ++i) {
        noise_signal.push_back(signal[i] + noise[i]);
    }

    // Работа с буффером

    // Корреляция с PSS
    auto pss = ofdm_mod.mapPSS(0);
    auto corr_pss = correlation(noise_signal, pss);

            float maxi = 0;
            for (int i = 0; i < corr_pss.size(); ++i) {
                if (corr_pss[i] > maxi) maxi = corr_pss[i];
            }
            std::cout << "Max corr pss: " << maxi << std::endl;

    // Находим индексы PSS
    auto indexs_pss = find_indexs_pss(corr_pss, 0.95);
            for (auto i : indexs_pss) {
                std::cout << "indexs_pss: " << i << std::endl;
            }

    OFDM_demod ofdm_demod;
    // Делим сигнал на слоты по индексам
    std::vector<double> corr;
    for (size_t i = 0; i < indexs_pss.size(); ++i) {
        auto only_signal =  extract_slots(noise_signal, indexs_pss, i);
                std::cout << only_signal.size() << std::endl;
        
        // Корреляция по CP в слоте
        auto corr_cp_arr = corr_cp(only_signal);
        // Находим индексы начала CP каждого символа
        auto indexs_cp = find_max_cp(corr_cp_arr);

                std::cout << "corr_cp_arr.size(): " << corr_cp_arr.size() << std::endl;
                std::cout << "CP_len: " << ofdm_demod.CP_len << std::endl;
                for(size_t k = 0; k < indexs_cp.size(); ++k) {
                    std::cout << "indexs_cp: " << indexs_cp[k] /*- (N_FFT + ofdm_demod.CP_len)*k*/ << std::endl;
                }
        corr = corr_cp_arr;
    }
    

    
    // for (auto i : index_cp) {
    //     std::cout << i << std::endl;
    // }

    

    std::string outputFile = "out.txt";
    saveCorr(corr, outputFile);

    return 0;
}
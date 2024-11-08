#include <iostream>
#include <fstream>
#include <vector>
#include <complex>
#include <string>
#include <cmath>
#include <omp.h>

#include "../../QAM/qam_mod.h"
#include "../../QAM/qam_demod.h"
#include "../../Segmenter/segmenter.h"
#include "../../OFDM/ofdm_mod.h"
#include "../../OFDM/ofdm_demod.h"
#include "../../OFDM/sequence.h"
#include "../../OFDM/fft/fft.h"
#include "../../File_converter/file_converter.h"

#define M_PI 3.14159265358979323846
using cd = std::complex<double>;

// g++ test.cpp  ../../File_converter/file_converter.cpp  ../../QAM/qam_mod.cpp ../../QAM/qam_demod.cpp ../../Segmenter/segmenter.cpp ../../OFDM/ofdm_mod.cpp ../../OFDM/ofdm_demod.cpp ../../OFDM/fft/fft.cpp ../../OFDM/sequence.cpp -fopenmp -o test && ./test

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
std::vector<cd> add_CFO(std::vector<cd>& signal, double CFO = 1500, uint32_t F_srate = 1920000) {
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

void saveD(const std::vector<double>& arr, const std::string& filename) {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cerr << "Не удалось открыть файл для записи." << std::endl;
        return;
    }

    for (const auto& value : arr) {
        outFile << value << "\n";
    }
}

void saveCD(const std::vector<cd>& arr, const std::string& filename) {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cerr << "Не удалось открыть файл для записи." << std::endl;
        return;
    }

    for (const auto& value : arr) {
        outFile << value << "\n";
    }
}

int main() {
    omp_set_num_threads(6);
    std::cout << "-----TX-----" << std::endl;
    Segmenter segmenter;
    // auto bits = generateRandBits(100, 2);
    auto bits = file2bits("test_file_in/арбуз арбуз.jpeg");
    auto segments = segmenter.segment(bits);
    segmenter.get_size_data_in_slot();
    segments = segmenter.scramble(segments);

    QAM_mod qam_mod;
    auto qpsk_mod = qam_mod.modulate(segments);

    OFDM_mod ofdm_mod;
    auto ofdm_data = ofdm_mod.modulate(qpsk_mod);

    double SNR_dB = 30.0;
    auto signal = pad_zeros(ofdm_data, 1000, 1000);
    signal = add_CFO(signal, 500);
    auto noise_signal = add_noise(signal, SNR_dB, 1);

    std::cout << "-----RX-----" << std::endl;
        auto start = std::chrono::high_resolution_clock::now();
    OFDM_demod ofdm_demod;
    auto demod_signal = ofdm_demod.demodulate(noise_signal);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Time for OFDM demodulation: " << duration.count() << " ms" << std::endl;

    QAM_demod qam_demod;
    auto demod_bits = qam_demod.demodulate(demod_signal);

    auto demod_bits_m = segmenter.reshape(demod_bits);
    demod_bits_m = segmenter.scramble(demod_bits_m);

    auto data = segmenter.extract_data(demod_bits_m);

    bits2file("test_file_out/", data);
    std::cout << "File saved" << std::endl;

    saveCD(demod_signal, "dem_sig.txt");
    saveCD(qpsk_mod[0], "qpsk.txt");
    //saveCD(signal, "signal.txt");

    return 0;
}
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
#include "../../OFDM/freq_offset.hpp"
#include "../../other/model_channel.h"
#include "../../other/plots.h"


using cd = std::complex<double>;
namespace plt = matplotlibcpp;


// g++ test.cpp -I/usr/include/python3.10 -lpython3.10 -fopenmp ../../other/plots.cpp ../../other/model_channel.cpp ../../File_converter/file_converter.cpp  ../../QAM/qam_mod.cpp ../../QAM/qam_demod.cpp ../../Segmenter/segmenter.cpp ../../OFDM/ofdm_mod.cpp ../../OFDM/ofdm_demod.cpp ../../OFDM/fft/fft.cpp ../../OFDM/sequence.cpp ../../OFDM/freq_offset.cpp -o test && ./test

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

std::vector<double> corr_cp_extended(const std::vector<cd>& slot_signal) {
    std::vector<double> corr(slot_signal.size(), 0.0);
    int CP_len = 9;
    OFDM_demod ofdm_demod;

    for (int i = 0; i <= slot_signal.size() - N_FFT - CP_len; ++i) {   
        std::vector<cd> first_win(slot_signal.begin() + i, slot_signal.begin() + i + CP_len);
        std::vector<cd> second_win(slot_signal.begin() + i + N_FFT, slot_signal.begin() + i + N_FFT + CP_len);
        
        std::vector<double> correlat = ofdm_demod.correlation(first_win, second_win);
        
        double correlat_cp = 0.0;
        for (auto var : correlat) correlat_cp += std::abs(var);
                
        corr[i] = correlat_cp;
    }

    return corr;
}

std::vector<cd> transmission(){
    std::cout << "-----TX-----" << std::endl;

    auto bits = generateRandBits(680*1, 2);
    // auto bits = string2bits("Hello, World! Привет, Мир! 1234567890");
    //auto bits = file2bits("test_file_in/арбуз арбуз.jpeg");
    
    Segmenter segmenter;
    auto segments = segmenter.segment(bits, 0); // Flag: 0 случайные биты, 1 - текст, 2 - файл
    segmenter.get_size_data_in_slot();
    segments = segmenter.scramble(segments);

    QAM_mod qam_mod;
    auto qpsk_mod = qam_mod.modulate(segments);

    OFDM_mod ofdm_mod;
    auto ofdm_data = ofdm_mod.modulate(qpsk_mod);

    return ofdm_data;
}

std::vector<cd> add_ch(std::vector<cd>& tx_signal) {
    double SNR_dB = 20.0;
    auto signal = pad_zeros(tx_signal, 1020, 1000);
    signal = add_CFO(signal, 2000);
    //signal = add_Channel(signal, {{1.0, 0.0}, {0.6, 0.1}, {0.4, -0.3}});
    auto noise_signal = add_noise(signal, SNR_dB, 1);

    return noise_signal;
}

std::vector<std::vector<uint8_t>> receive(std::vector<cd>& noise_signal) {
    std::cout << "-----RX-----" << std::endl;

        auto start = std::chrono::high_resolution_clock::now();
    OFDM_mod ofdm_mod;
    std::vector<std::complex<double>> noise_signal_cfo;
    frequency_correlation(ofdm_mod.mapPSS(), noise_signal, 15000, noise_signal_cfo, 1920000);

    OFDM_demod ofdm_demod;
    auto demod_signal = ofdm_demod.demodulate(noise_signal_cfo);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Time for OFDM demodulation: " << duration.count() << " ms" << std::endl;

    QAM_demod qam_demod;
    auto demod_bits = qam_demod.demodulate(demod_signal);

    Segmenter segmenter;
    auto demod_bits_m = segmenter.reshape(demod_bits);
    demod_bits_m = segmenter.scramble(demod_bits_m);

    // auto data = segmenter.extract_data(demod_bits_m);
    // auto flag = segmenter.extract_flag(demod_bits_m);

    // if      (flag == 1) bits2string(data);
    // else if (flag == 2) bits2file("test_file_out/", data);
    // //else if (flag == 3) sistem_inf_rx(data);

    return demod_bits_m;
}

void sistem_inf_rx(std::vector<uint8_t>& data){
    // Первыt 8 бит
    if (data.size() < 8) {
        std::cerr << "Недостаточно данных для извлечения числа." << std::endl;
        return;
    }
    uint8_t message = 0;
    for (int i = 0; i < 8; ++i) {
        message |= (data[i] << (7 - i));
    }

    std::cout << "Извлеченное число: 0x" << std::hex << static_cast<int>(message) << std::dec << std::endl;

    switch (message)
    {
    case 0:
        // 0 - Тут кто-нибудь есть?
        break;
    case 1:
        // 1 - Я здесь!
        break;
    case 2:
        // 2 Соединение установлено 
        break;
    case 3:
        // 3 - Конец связи
        break;
    
    case 10:
        // 10 - Принято без ошибок (ACK)
        break;
    case 15:
        // 15 - Приняты слоты с ошибкой (NACK)
        break;

    default:
        break;
    }
}

int main() {
    omp_set_num_threads(8);
    
    auto ofdm_data = transmission();
    auto noise_signal = add_ch(ofdm_data);
    auto demod_bits_m = receive(noise_signal);

    Segmenter segmenter;
    auto data = segmenter.extract_data(demod_bits_m);
    auto flag = segmenter.extract_flag(demod_bits_m);

    if      (flag == 1) bits2string(data);
    else if (flag == 2) bits2file("test_file_out/", data);

    // spectrogram_plot(noise_signal_cfo, "noise_signal_cfo");
    // cool_scatter(demod_signal, "demod_signal");
    // cool_plot(corr, "corr");
    show_plot();

    return 0;
}
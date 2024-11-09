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
#include "model_channel.h"


using cd = std::complex<double>;


// g++ test.cpp  model_channel.cpp ../../File_converter/file_converter.cpp  ../../QAM/qam_mod.cpp ../../QAM/qam_demod.cpp ../../Segmenter/segmenter.cpp ../../OFDM/ofdm_mod.cpp ../../OFDM/ofdm_demod.cpp ../../OFDM/fft/fft.cpp ../../OFDM/sequence.cpp ../../OFDM/freq_offset.cpp -fopenmp -o test && ./test

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
    omp_set_num_threads(8);
    std::cout << "-----TX-----" << std::endl;

    // auto bits = generateRandBits(100, 2);
    auto bits = file2bits("test_file_in/арбуз арбуз.jpeg");
    
    Segmenter segmenter;
    auto segments = segmenter.segment(bits);
    segmenter.get_size_data_in_slot();
    segments = segmenter.scramble(segments);

    QAM_mod qam_mod;
    auto qpsk_mod = qam_mod.modulate(segments);

    OFDM_mod ofdm_mod;
    auto ofdm_data = ofdm_mod.modulate(qpsk_mod);

    double SNR_dB = 20.0;
    auto signal = pad_zeros(ofdm_data, 2000, 1000);
    signal = add_CFO(signal, 6000);
    auto noise_signal = add_noise(signal, SNR_dB, 1);


    std::cout << "-----RX-----" << std::endl;

        auto start = std::chrono::high_resolution_clock::now();
    std::vector<std::complex<double>> noise_signal_cfo;
    frequency_correlation(ofdm_mod.mapPSS(0), noise_signal, 15000, noise_signal_cfo, 1920000);

    OFDM_demod ofdm_demod;
    auto demod_signal = ofdm_demod.demodulate(noise_signal_cfo);
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


    //saveCD(demod_signal, "test_file_out/dem_sig.txt");
    // saveCD(qpsk_mod[0], "qpsk.txt");
    // saveCD(signal, "signal.txt");

    return 0;
}
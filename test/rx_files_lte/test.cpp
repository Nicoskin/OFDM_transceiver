#include <iostream>
#include <fstream>
#include <vector>
#include <complex>
#include <string>

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




// g++ test.cpp -I/usr/include/python3.10 -lpython3.10 -fopenmp ../../other/plots.cpp ../../other/model_channel.cpp ../../File_converter/file_converter.cpp  ../../QAM/qam_mod.cpp ../../QAM/qam_demod.cpp ../../Segmenter/segmenter.cpp ../../OFDM/ofdm_mod.cpp ../../OFDM/ofdm_demod.cpp ../../OFDM/fft/fft.cpp ../../OFDM/sequence.cpp ../../OFDM/freq_offset.cpp -o test && ./test


using cd = std::complex<double>;

std::vector<std::complex<double>> readComplexVector(const std::string& realFile, const std::string& imagFile) {
    std::ifstream realStream(realFile);
    std::ifstream imagStream(imagFile);

    std::vector<std::complex<double>> complexVector;
    
    if (!realStream.is_open() || !imagStream.is_open()) {
        std::cerr << "Не удалось открыть файлы." << std::endl;
        return complexVector;
    }
    
    double realValue, imagValue;
    while (realStream >> realValue && imagStream >> imagValue) {
        complexVector.emplace_back(realValue, imagValue);
    }

    return complexVector;
}

void saveComplexVector(const std::vector<std::complex<double>>& complexVector, const std::string& filename) {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cerr << "Не удалось открыть файл для записи." << std::endl;
        return;
    }

    for (const auto& value : complexVector) {
        outFile << value.real() << " " << value.imag() << "\n";
    }
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

std::vector<std::complex<double>> readComplexNumbersFromFile(const std::string& filename) {
    std::vector<std::complex<double>> result;
    std::ifstream file(filename);

    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file: " + filename);
    }

    std::string line;
    while (std::getline(file, line)) {
        // Remove unwanted characters
        line.erase(remove(line.begin(), line.end(), '('), line.end());
        line.erase(remove(line.begin(), line.end(), ')'), line.end());

        std::istringstream iss(line);
        double real, imag;
        char plus_or_minus, i;

        // Parse the real and imaginary parts
        if (iss >> real >> plus_or_minus >> imag >> i && (plus_or_minus == '+' || plus_or_minus == '-')) {
            if (plus_or_minus == '-') {
                imag = -imag;
            }
            result.emplace_back(real, imag);
        } else {
            throw std::runtime_error("Invalid format in file");
        }
    }

    file.close();
    return result;
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

void pss_spec(const std::vector<cd>& signal, int first_ind_pss) {
    std::vector<cd> all;
    for(size_t i=0; i < 50; i++){
    std::vector<cd> one_symb(signal.begin() + first_ind_pss + i, signal.begin() + first_ind_pss + i + N_FFT);
    // auto ff = fft(one_symb);
    // ff = fftshift(ff);
    all.insert(all.end(), one_symb.begin(), one_symb.end());
    }
    spectrogram_plot(all);
}

int main() { 
    // Чтение данных из файлов и создание вектора комплексных чисел
    //std::vector<cd> complexVector = readComplexVector("rx_imag_file.txt", "rx_real_file.txt");
    std::vector<cd> complexVector = readComplexNumbersFromFile("rx_file_complex_fs_1.92.txt");
    //complexVector = std::vector<std::complex<double>>(begin(complexVector)+45000, begin(complexVector) + 55000);
    complexVector = std::vector<std::complex<double>>(begin(complexVector)+1000, begin(complexVector) + 2500);
    //std::vector<cd> PSS_PBCH(begin(complexVector)+11000, begin(complexVector)+12500); // 4
 
    OFDM_mod ofdm_mod;
    OFDM_demod ofdm_demod;
    auto pss = ofdm_mod.mapPSS(1);
    auto corr = ofdm_demod.correlation(complexVector, pss);
    double max_corr = *std::max_element(begin(corr), end(corr));
    int max_corr_ind = std::distance(begin(corr), std::max_element(begin(corr), end(corr)));
    std::cout << "Ind PSS: " << max_corr_ind << std::endl;
    std::cout << "Max PSS: " << max_corr << std::endl;
    // std::vector<std::complex<double>> noise_signal_cfo;
    // frequency_correlation(ofdm_mod.mapPSS(1), complexVector, 15000, noise_signal_cfo, 1920000);
    pss_spec(complexVector, 720);
    pss_spec(complexVector, 749-128-32+11);
    
    std::vector<cd> only_sss = std::vector<cd>(begin(complexVector)+749-128-32+11, begin(complexVector)+749-128-32+11+N_FFT);
    auto sss_fft = fft(only_sss);
    sss_fft = fftshift(sss_fft);
    cool_plot(sss_fft);

    double max_sss = 0.0;
    int n_sss = 0;
    int ind_sss = 0;
    for(int N_id = 0; N_id < 168; N_id++){
        auto sss = ofdm_mod.mapSSS(N_id, 5);
        auto corr_sss = ofdm_demod.correlation(sss_fft, sss);
        double max_corr_sss = *std::max_element(begin(corr_sss), end(corr_sss));
        //std::cout << max_corr_sss << " ";
        n_sss = max_corr_sss > max_sss ? N_id : n_sss;
        ind_sss = max_corr_sss > max_sss ? std::distance(begin(corr_sss), std::max_element(begin(corr_sss), end(corr_sss))) : ind_sss;
        max_sss = max_corr_sss > max_sss ? max_corr_sss : max_sss;
    }
    std::cout << std::endl;
    std::cout << "n_sss: " << n_sss << std::endl;
    std::cout << "max_sss: " << max_sss << std::endl;
    std::cout << "ind_sss: " << ind_sss << std::endl;
    /*
    // std::cout << "N_id: " << n_sss*3 + 1 << std::endl;

    // auto corr_sss = ofdm_demod.correlation(complexVector, sss);
    // double max_corr_sss = *std::max_element(begin(corr_sss), end(corr_sss));
    // int max_corr_ind_sss = std::distance(begin(corr_sss), std::max_element(begin(corr_sss), end(corr_sss)));
    // cool_plot(corr_sss, "Correlation with SSS");
    std::vector<cd> only_pss(begin(complexVector)+730, begin(complexVector)+730+N_FFT);
    auto pss_fft = fft(only_pss);
    pss_fft = fftshift(pss_fft);
    cool_scatter(pss_fft, "PSS FFT");
    for(auto & var : pss_fft) var = std::abs(var);
    cool_plot(pss_fft, "PSS FFT");


    std::vector<cd> only_sss(begin(complexVector)+593, begin(complexVector)+593+N_FFT);
    auto sss_fft = fft(only_sss);
    sss_fft = fftshift(sss_fft);
    cool_scatter(sss_fft, "SSS FFT");
    //for(auto & var : sss_fft) var = std::abs(var);
    cool_plot(sss_fft, "SSS FFT");

    auto corr_cp = corr_cp_extended(complexVector);
    cool_plot(corr_cp, "Correlation with CP");


    // cool_plot(corr, "Correlation with PSS");
    cool_plot(complexVector);
    spectrogram_plot(complexVector);
    // spectrogram_plot(noise_signal_cfo);
    */


    show_plot();

    return 0;
}
#include <iostream>
#include <fstream>
#include <vector>
#include <complex>
#include <string>

// #include "../../QAM/qam_mod.h"
// #include "../../QAM/qam_demod.h"
// #include "../../Segmenter/segmenter.h"
// #include "../../OFDM/ofdm_mod.h"
// #include "../../OFDM/ofdm_demod.h"
// #include "../../OFDM/sequence.h"
// #include "../../OFDM/fft/fft.h"
// #include "../../File_converter/file_converter.h"
// #include "../../OFDM/freq_offset.hpp"
// #include "../../other/model_channel.h"
// #include "../../other/plots.h"

#include "func_for_real_lte.h"




// g++ test.cpp -I/usr/include/python3.10 -lpython3.10 -fopenmp func_for_real_lte.cpp ../../other/plots.cpp ../../other/model_channel.cpp ../../File_converter/file_converter.cpp  ../../QAM/qam_mod.cpp ../../QAM/qam_demod.cpp ../../Segmenter/segmenter.cpp ../../OFDM/ofdm_mod.cpp ../../OFDM/ofdm_demod.cpp ../../OFDM/fft/fft.cpp ../../OFDM/sequence.cpp ../../OFDM/freq_offset.cpp -o test && ./test


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

void smeared(const std::vector<cd>& signal, int first_ind_pss) {
    std::vector<cd> all;
    for(size_t i=0; i < 200; i++){
    std::vector<cd> one_symb(signal.begin() + first_ind_pss + i, signal.begin() + first_ind_pss + i + N_FFT);
    // auto ff = fft(one_symb);
    // ff = fftshift(ff);
    all.insert(all.end(), one_symb.begin(), one_symb.end());
    }
    spectrogram_plot(all);
}

int main() { 
    std::vector<cd> complexVector = readComplexNumsFromFile("rx_ue_3sdr_pci31.txt");

    OFDM_mod ofdm_mod;
    OFDM_demod ofdm_demod;
    complexVector = std::vector<cd>(complexVector.begin() + 20000, complexVector.end());
    auto vec = calculate_pci(complexVector);
    int pci = vec[0];
    int index_first_pss = vec[1];
    if (pci < 0) {
        std::cerr << "PCI не найден." << std::endl;
        return -1;
    }
    std::cout << "PCI: " << pci;
    std::cout << "  Index first PSS: " << index_first_pss << std::endl;

    // Убираем CP
    std::vector<cd> time_pbch_no_cp = time_pbch_without_cp(complexVector, index_first_pss);

    // Гунерируем пилоты
    std::vector<std::vector<std::vector<cd>>> refs;
    refs.resize(20, std::vector<std::vector<cd>>(7, std::vector<cd>(6 * 2, cd(0, 0))));
    gen_pilots_siq(refs, pci, false);

    // Переходим в частотную область
    std::vector<cd> freq_pbch;
    for(size_t i = 0; i < 5; i++){
        auto fft_res = fft(std::vector<cd>(time_pbch_no_cp.begin() + N_FFT*i, time_pbch_no_cp.begin() + N_FFT*(i+1)));
        fft_res = fftshift(fft_res);
        freq_pbch.insert(freq_pbch.end(), fft_res.begin(), fft_res.end());
    }

    auto inter_H_0 = interpolated_channel_estimator(std::vector<cd>(freq_pbch.begin()        , freq_pbch.begin()+N_FFT  ), 1, 0, refs, true);
    auto inter_H_4 = interpolated_channel_estimator(std::vector<cd>(freq_pbch.begin()+N_FFT*4, freq_pbch.begin()+N_FFT*5), 1, 4, refs, false);
    auto inter_H_1_3 = interpolating_H_0to4_symb(inter_H_0, inter_H_4);
        // cool_plot(inter_H_0, "Interpolated H 0");
        // cool_plot(std::vector<cd>(freq_pbch.begin(),     freq_pbch.begin()+N_FFT), "H 0", "-o");
        // cool_plot(inter_H_4, "Interpolated H 4");
        // cool_plot(std::vector<cd>(freq_pbch.begin()+512, freq_pbch.begin()+640), "H 4", "-o");
        // cool_plot(inter_H_1_3, "Interpolated H 1-3");

    // Делим на оценку канала
    std::vector<cd> pbch_symb_divide_CH(N_FFT*4, 0);
    for (int k_s = 0; k_s < N_FFT*4; k_s++) { 
        if (k_s < N_FFT) {
            if (std::real(inter_H_0[k_s]) == 0) pbch_symb_divide_CH[k_s] = 0;
            else pbch_symb_divide_CH[k_s] = freq_pbch[k_s] / inter_H_0[k_s]; 
        }
        else {
            if (std::real(inter_H_1_3[k_s - N_FFT]) == 0) pbch_symb_divide_CH[k_s] = 0;
            else pbch_symb_divide_CH[k_s] = freq_pbch[k_s] / inter_H_1_3[k_s - N_FFT]; 
        }
        //pbch_symb_divide_CH[k_s] = freq_pbch[k_s];
    }

    cool_plot(pbch_symb_divide_CH, "pbch_symb", "-o");
    //std::cout << "pbch_symb_divide_CH: " << pbch_symb_divide_CH[356] << pbch_symb_divide_CH[484] << std::endl;


    show_plot();

    return 0;
}
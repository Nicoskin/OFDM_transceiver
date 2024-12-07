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
    std::vector<cd> complexVector = readComplexNumbersFromFile("rx_ue_3sdr_pci31.txt");
    OFDM_mod ofdm_mod;
    OFDM_demod ofdm_demod;
    spectrogram_plot(complexVector, "Received signal");

    if (complexVector.size() < 19200) {
        std::cerr << "Слишком мало данных для обработки." << std::endl;
        return -1;
    }
    std::vector<cd> one_frame(complexVector.begin(), complexVector.begin() + 19200);

    std::vector<int> indices_pss;
    int pss_root = 0;
    for(size_t root = 0; root < 3; root++) {
        auto pss = ofdm_mod.mapPSS(root);
        auto corr = ofdm_demod.correlation(one_frame, pss);
        for (size_t i = 0; i < corr.size(); ++i) {
            if (corr[i] > 0.8) {
                indices_pss.push_back(i);
            }
        }
        if (indices_pss.size() == 2) {
            pss_root = root;
            std::cout << "PSS: " << pss_root << std::endl;
            std::cout << "Indices: " << indices_pss[0] << " " << indices_pss[1] << std::endl;
            //cool_plot(corr, "Correlation with PSS", "-");
            break;
        }
    }
    // Частотная по PSS
    std::vector<cd> signal_cfo;
    frequency_correlation(ofdm_mod.mapPSS(pss_root), one_frame, 15000, signal_cfo, 1920000);
    one_frame = signal_cfo;

    int sss_root = -1;
    int index_first_pss = 0;
    auto sss_fft = fft(std::vector<cd>(one_frame.begin() + indices_pss[0] - 137, one_frame.begin() + indices_pss[0] - 9));
    sss_fft = fftshift(sss_fft);

    for(size_t subframe = 0; subframe <= 5; subframe += 5){
        for(size_t sss = 0; sss < 168; sss++) {
            auto sss_freq = generate_sss(sss*3 + pss_root, subframe, true);
            auto corr = ofdm_demod.correlation(sss_fft, sss_freq);
            if (*std::max_element(corr.begin(), corr.end()) > 0.9) {
                sss_root = sss;
                std::cout << "SSS: " << sss << std::endl;
                //cool_plot(corr, "Correlation with SSS", "-");
                break;
            }
        }
        if((sss_root != -1) && (subframe == 0)) {
            index_first_pss = indices_pss[0];
            break;
        }
        else index_first_pss = indices_pss[1];
    }
    std::cout << "Index first PSS: " << index_first_pss << std::endl;
    show_plot();

    return 0;
}
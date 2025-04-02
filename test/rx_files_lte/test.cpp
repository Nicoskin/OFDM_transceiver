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
#include "pbch.h"
//#include "viterbi.h"
// #include "coders.h"




// g++ test.cpp -I/usr/include/python3.10 -lpython3.10 -fopenmp func_for_real_lte.cpp pbch.cpp ../../other/plots.cpp ../../other/model_channel.cpp ../../File_converter/file_converter.cpp  ../../QAM/qam_mod.cpp ../../QAM/qam_demod.cpp ../../Segmenter/segmenter.cpp ../../OFDM/ofdm_mod.cpp ../../OFDM/ofdm_demod.cpp ../../OFDM/fft/fft.cpp ../../OFDM/sequence.cpp ../../OFDM/freq_offset.cpp -o test && ./test


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
    complexVector = std::vector<cd>(complexVector.begin() + 31000, complexVector.end());
    
    // PCI
    auto vec_pci = calculate_pci(complexVector);
    if (vec_pci[0] < 0) {
        std::cerr << "PCI не найден." << std::endl;
        return -1;
    }

    // PBCH freq
    auto pbch_no_zeros = preparing_pbch(complexVector, vec_pci);

    // PBCH -> MIB + CRC
    auto pbch_mib_and_crc = pbch_decode(pbch_no_zeros, vec_pci[0]);
    for (auto bit : pbch_mib_and_crc) {
            std::cout << (int)bit;
    }
    std::cout << std::endl;

    if (!pbch_mib_and_crc.empty())
        pbch_mib_unpack(pbch_mib_and_crc);

    //show_plot();

    return 0;
}
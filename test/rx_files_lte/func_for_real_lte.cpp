#include <iostream>
#include <fstream>
#include <vector>
#include <complex>
#include <string>

#include "func_for_real_lte.h"

using cd = std::complex<double>;

std::vector<std::complex<double>> readComplexNumsFromFile(const std::string& filename) {
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

std::vector<int> search_pss(std::vector<cd>& complexVector){
    OFDM_mod ofdm_mod;
    OFDM_demod ofdm_demod;

    if (complexVector.size() < 19200) {
        std::cerr << "Слишком мало данных для обработки." << std::endl;
        return {};
    }
    std::vector<cd> one_frame(complexVector.begin(), complexVector.begin() + 19200);

    std::vector<int> indices_pss;
    int pss_root = 0;
    double threshold_pss = 0.73; // Менять

    for(size_t root = 0; root < 3; root++) {
        auto pss = ofdm_mod.mapPSS(root);
        auto corr = ofdm_demod.correlation(one_frame, pss);
        for (size_t i = 0; i < corr.size(); ++i) {
            if (corr[i] > threshold_pss) {
                if (indices_pss.empty()){
                    indices_pss.push_back(i);
                    continue;
                }
                else if (i - indices_pss.back() > 100)
                    indices_pss.push_back(i);
                else if (corr[i] > indices_pss.back())
                    indices_pss.back() = i;
            }
        }
        if (indices_pss.size() == 2) {
            pss_root = root;
            break;
        }
    }
    return {pss_root, indices_pss[0], indices_pss[1]};
}

std::vector<int> calculate_pci(std::vector<cd>& complexVector){
    OFDM_mod ofdm_mod;
    OFDM_demod ofdm_demod;

    // PSS
    auto vec_pss = search_pss(complexVector);
    auto pss_root = vec_pss[0];
    auto indices_pss = std::vector<int>(vec_pss.begin() + 1, vec_pss.end());
    //std::cout << "PSS: " << pss_root << std::endl;

    std::vector<cd> one_frame(complexVector.begin(), complexVector.begin() + 19200);
    
    // Частотная по PSS
    std::vector<cd> signal_cfo;
    frequency_correlation(ofdm_mod.mapPSS(pss_root), one_frame, 15000, signal_cfo, 1920000);
    complexVector = signal_cfo;
    one_frame = signal_cfo;

    int CP_len;
    if (CP_LEN == 0) CP_len = N_FFT / 12.8;
    else CP_len = N_FFT / 4;
    
    int sss_root = -1;
    int index_first_pss = 0;
    std::vector<cd> sss_fft;
    if (CP_LEN == 0)
        sss_fft = fft(std::vector<cd>(one_frame.begin() + indices_pss[0] - (N_FFT+CP_len*0.9), one_frame.begin() + indices_pss[0] - CP_len*0.9));
    else
        sss_fft = fft(std::vector<cd>(one_frame.begin() + indices_pss[0] - (N_FFT+CP_len), one_frame.begin() + indices_pss[0] - CP_len));
    sss_fft = fftshift(sss_fft);

    std::vector<cd> channel_estimation;
    auto pss = ofdm_mod.mapPSS(pss_root);
    pss = fft(pss);
    pss = fftshift(pss);
    auto pss_fft = fft(std::vector<cd>(one_frame.begin() + indices_pss[0], one_frame.begin() + indices_pss[0] + N_FFT));
    pss_fft = fftshift(pss_fft);
    for (size_t i = 0; i < N_FFT; i++) {
        if (std::real(std::abs(pss[i])) < 0.00000001) channel_estimation.push_back(0);
        else
        channel_estimation.push_back(pss_fft[i] / pss[i]);
    }
    for (size_t i = 0; i < N_FFT; i++) {
        if (std::real(channel_estimation[i]) != 0)
            sss_fft[i] = sss_fft[i] / channel_estimation[i];
        else
            sss_fft[i] = 0;
    }

    double threshold_sss = 0.65;
    for(size_t subframe = 0; subframe <= 5; subframe += 5){
        for(size_t sss = 0; sss < 168; sss++) {
            auto sss_freq = generate_sss(sss*3 + pss_root, subframe, true);
            auto corr = ofdm_demod.correlation(sss_fft, sss_freq);
            if (*std::max_element(corr.begin(), corr.end()) > threshold_sss) {
                sss_root = sss;
                break;
            }
        }
        if((sss_root != -1) && (subframe == 0)) {
            index_first_pss = indices_pss[0];
            break;
        }
        else index_first_pss = indices_pss[1];
    }

    std::vector<int> pci(2, 0);
    pci[0] = sss_root*3 + pss_root;
    pci[1] = index_first_pss;
    return pci;

}

std::vector<cd> interpolated_channel_estimator(const std::vector<cd> signal, int n_slot, int n_symb, std::vector<std::vector<std::vector<cd>>> refs, bool zero_sym) {
    std::vector<cd> H_channel(N_PILOTS);
    std::vector<cd> interpolated_signal(N_FFT);
    OFDM_mod ofdm_mod;
    OFDM_Data_S data = OFDM_Data_S(31, false);

    std::vector<int> pilots_ind;
    if (zero_sym) pilots_ind = data.pilot_indices;
    else pilots_ind = data.pilot_indices_shifted;

    auto pilot_val = refs[n_slot][n_symb];
    // std::cout << "INDICES: ";
    // for (auto &ind : pilots_ind) {
    //     std::cout << ind << " ";
    // }
    // std::cout << std::endl;
    int k = 0;
    for (int idx : pilots_ind) {
        //H_channel[k++] = signal[idx] / pilot_val[k];
        interpolated_signal[idx] = signal[idx] / pilot_val[k++];  // Размещение оценок канала на индексах пилотов
    }
    // Интерполяция между каждой парой контрольных значений в пределах диапазона
    
    // Начальный индекс данных
    const int start_data_idx = G_SUBCAR / 2 + 1;
    const int end_range = N_FFT - G_SUBCAR / 2;
    // Интерполяция между каждой парой контрольных значений в пределах диапазона
    for (size_t i = 0; i < pilots_ind.size() - 1; ++i) {
        int start_idx = pilots_ind[i];
        int end_idx = std::min(pilots_ind[i + 1], end_range);  // end_idx не превышает заданный диапазон

        cd start_val = interpolated_signal[start_idx];
        cd end_val = interpolated_signal[end_idx];

        for (int j = start_idx + 1; j < end_idx; ++j) {
            double alpha = static_cast<double>(j - start_idx) / (end_idx - start_idx);
            interpolated_signal[j] = start_val * (1.0 - alpha) + end_val * alpha;
        }
    }

    // Продолжает значения до первого пилота
    for (size_t i = start_data_idx; i < pilots_ind[0]; ++i){
        interpolated_signal[i] = interpolated_signal[pilots_ind[0]];
    }

    // Продолжает значения после последнего пилота
    int last_pilot = pilots_ind.back();
    if (last_pilot < start_data_idx) {
        last_pilot = start_data_idx;
    }
    cd last_val = interpolated_signal[last_pilot];
    for (int j = last_pilot + 1; j < end_range; ++j) {
        interpolated_signal[j] = last_val;
    }

    return interpolated_signal;
}

std::vector<cd> interpolating_H_0to4_symb(std::vector<cd> H_0, std::vector<cd> H_4) {
    std::vector<cd> H_0to4(N_FFT * 3);
    for (int symb = 0; symb < 3; symb++) {
        double alpha = (symb + 1) / 3.0; // interpolation factor: 1/4, 2/4, 3/4
        for (int i = 0; i < N_FFT; ++i) {
            H_0to4[symb * N_FFT + i] = H_0[i] * (1.0 - alpha) + H_4[i] * alpha;
        }
    }
    return H_0to4;
}

std::vector<cd> time_pbch_without_cp(std::vector<cd> complexVector, int index_first_pss) {
    int CP_len;
    if (CP_LEN == 0) CP_len = N_FFT / 12.8;
    else CP_len = N_FFT / 4;

    int len_5_ofdm_with_cp = 4 * (N_FFT + CP_len*0.9) + (N_FFT + CP_len);
    std::vector<cd> time_pbch = std::vector<cd>(complexVector.begin() + index_first_pss + N_FFT, complexVector.begin() + index_first_pss + N_FFT + len_5_ofdm_with_cp);
    std::vector<cd> time_pbch_without_cp;
    for(size_t i = 0; i < 5; i++){
        time_pbch_without_cp.insert(time_pbch_without_cp.end(), time_pbch.begin() + 1 + N_FFT*i + CP_len*0.9*(i+1), time_pbch.begin()+1 + N_FFT*(i+1) + CP_len*0.9*(i+1) );
    }
    return time_pbch_without_cp;
}

std::vector<cd> preparing_pbch(std::vector<cd> complexVector, std::vector<int> vec_pci) {
    int pci = vec_pci[0];
    int index_first_pss = vec_pci[1];

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

    auto data = OFDM_Data_S(pci); 
    std::vector<cd> pbch_no_zeros;
    for(size_t symbol = 0; symbol < 4; symbol++){
        if (symbol <= 1){
            for(auto ind : data.pbch_indices){
                pbch_no_zeros.push_back(pbch_symb_divide_CH[symbol*N_FFT + ind]);
            }
        }
        else{
            for(auto ind : data.data_indices_noPilots){
                pbch_no_zeros.push_back(pbch_symb_divide_CH[symbol*N_FFT + ind]);
            }
        }
    }

    return pbch_no_zeros;
}

std::vector<uint8_t> pbch_decode(std::vector<cd> freq_pbch_no_zeros, int pci) {
    QAM_demod qam_demod(2);
    auto pbch_bits = qam_demod.demodulate(freq_pbch_no_zeros);

    std::vector<uint8_t> scrambled_bits = scrambling(pbch_bits, pci, 0);

    auto de_rate_matched_bits = de_rate_match(scrambled_bits, 120);

    auto decoded = decode(de_rate_matched_bits);

    auto crc = check_crc(decoded);

    if (crc != 1) 
        return {};
    else 
        return decoded;
}

void pbch_mib_unpack(std::vector<uint8_t> pbch_mib_and_crc) {
    std::cout << "MIB: " << std::endl;
    // bandwidth
    int bandwidth = pbch_mib_and_crc[0]*4 + pbch_mib_and_crc[1]*2 + pbch_mib_and_crc[2];
    std::cout << "  bandwidth: " ;
    switch (bandwidth)
    {
    case 0:
        std::cout << "n6" << std::endl;
        break;
    case 1:
        std::cout << "n15" << std::endl;
        break;
    case 2:
        std::cout << "n25" << std::endl;
        break;
    case 3:
        std::cout << "n50" << std::endl;
        break;
    case 4:
        std::cout << "n75" << std::endl;
        break;
    case 5:
        std::cout << "n100" << std::endl;
        break;

    default:
        break;
    }

    // phich_conf
    std::cout << "  PHICH Config: " << std::endl;
    int phich_conf =  pbch_mib_and_crc[3]*4 + pbch_mib_and_crc[4]*2 + pbch_mib_and_crc[5];
    if ((phich_conf<<2) == 0)
        std::cout << "      PHICH-Duration: "<< "normal" << std::endl;
    else
        std::cout << "      PHICH-Duration: "<< "extended" << std::endl;

    if ((phich_conf<<1) == 0)
        std::cout << "      PHICH-Resource: "<< "1" << std::endl;
    else if ((phich_conf<<1) == 1)
        std::cout << "      PHICH-Resource: "<< "1/2" << std::endl;
    else if ((phich_conf<<1) == 2)
        std::cout << "      PHICH-Resource: "<< "1/6" << std::endl;
    else
        std::cout << "      PHICH-Resource: "<< "2" << std::endl;

    // System Frame Number
    std::cout << "  System Frame Number: ";
    int system_frame_number = 0;
    for (int i = 0; i < 8; i++){
        system_frame_number += pbch_mib_and_crc[i+6]*pow(2,9-i);
    }
    std::cout << system_frame_number << " | " ;
    for (int i = 0; i < 8; i++){
        std::cout << (int)pbch_mib_and_crc[i+6];
    }
    std::cout << std::endl;

    // Reserved bits
    std::cout << "  Reserved bits: ";
    int reserved_bits = 0;
    for (int i = 0; i < 10; i++){
        reserved_bits += pbch_mib_and_crc[i+14]*pow(2,11-i);
    }
    for (int i = 0; i < 10; i++){
        std::cout << (int)pbch_mib_and_crc[i+14];
    }
    std::cout << std::endl;
    std::cout << std::endl;

}



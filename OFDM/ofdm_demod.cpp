#include "ofdm_demod.h"
#include "fft/fft.h"

#include "ofdm_mod.h"

#include <iostream>

namespace {
    using cd = std::complex<double>;
}

OFDM_demod::OFDM_demod(){
    if (CP_LEN == 0) CP_len = N_FFT / 12.8;
    else CP_len = N_FFT / 4;
};

std::vector<cd> OFDM_demod::demodulate(const std::vector<cd>& signal) {
    std::vector<cd> demod_signal;
    OFDM_mod ofdm_mod;
    auto pss = ofdm_mod.mapPSS(0);
    auto corr_pss = correlation(signal, pss);
    auto data_indices = ofdm_mod.data_indices;
            // float maxi = 0;
            // for (int i = 0; i < corr_pss.size(); ++i) {
            //     if (corr_pss[i] > maxi) maxi = corr_pss[i];
            // }
            // std::cout << "Max corr pss: " << maxi << std::endl;

    auto indexs_pss = find_ind_pss(corr_pss, 0.87);

    // Цикл по каждому слоту
    for (size_t n_slot = 0; n_slot < indexs_pss.size(); ++n_slot) {
        auto one_slot = extract_slots(signal, indexs_pss, n_slot);
        
        auto corr_cp_arr = corr_cp(one_slot);
        auto indexs_cp = find_ind_cp(corr_cp_arr);

        // Цикл по каждому символу в слоте
        for(size_t n_symb = 0; n_symb < OFDM_SYM_IN_SLOT; n_symb++){
            auto one_symb = extract_symb(one_slot, indexs_cp, n_symb);

            auto one_symb_freq = fft(one_symb);
            one_symb_freq = fftshift(one_symb_freq);

            auto inter_H = interpolated_H(one_symb_freq, n_slot, n_symb);

            // Деление на канал
            for(int k_s = 0; k_s < N_FFT; k_s++){
                one_symb_freq[k_s] = one_symb_freq[k_s] / inter_H[k_s];
            }

            // Сохраняем только данные
            for(auto ind : data_indices){
                demod_signal.push_back(one_symb_freq[ind]);
            }
        }
    }

    return demod_signal;
}

std::vector<cd> OFDM_demod::convolve_fft(const std::vector<cd>& vec1, const std::vector<cd>& vec2) {
    int n1 = vec1.size(); 
    int n2 = vec2.size(); 
    int n = nearest_power_of_two(n1 + n2 - 1); // Определяем размер результата корреляции как ближайшую степень двойки

    std::vector<cd> padded_vec1(n, 0);
    std::vector<cd> padded_vec2(n, 0);

    // Копируем данные из vec1 в padded_vec1
    std::copy(vec1.begin(), vec1.end(), padded_vec1.begin());
    std::copy(vec2.rbegin(), vec2.rend(), padded_vec2.begin());

    // Берем комплексно-сопряженное значение второго вектора для корреляции
    for (int i = 0; i < n2; ++i) {
        padded_vec2[i] = std::conj(padded_vec2[i]);
    }

    // Выполняем FFT для обоих векторов
    auto fft_vec1 = fft(padded_vec1);
    auto fft_vec2 = fft(padded_vec2);

    // Элемент-wise умножение в частотной области
    std::vector<cd> fft_product(n);
    for (int i = 0; i < n; ++i) {
        fft_product[i] = fft_vec1[i] * fft_vec2[i]; // Умножаем элементы FFT двух векторов
    }

    // Выполняем IFFT, чтобы получить результат корреляции
    std::vector<cd> result = ifft(fft_product);

    return result; // Возвращаем результат корреляции
}

std::vector<double> OFDM_demod::correlation(const std::vector<cd>& y1, const std::vector<cd>& y2) {
    size_t len1 = y1.size();
    size_t len2 = y2.size();
    size_t maxShift = len1 - len2 + 1;  // Максимальный сдвиг для корреляции
    std::vector<double> result(maxShift, 0.0);

    // Предварительные вычисления нормировочных коэффициентов
    double normY2 = 0.0;
    for (const auto& v : y2) {
        normY2 += std::norm(v);
    }
    normY2 = std::sqrt(normY2);

    // Цикл по сдвигам
    for (size_t shift = 0; shift < maxShift; ++shift) {
        std::complex<double> sum(0.0, 0.0);
        double normY1 = 0.0;

        // Вычисление корреляции на текущем сдвиге
        for (size_t i = 0; i < len2; ++i) {
            sum += y1[i + shift] * std::conj(y2[i]);
            normY1 += std::norm(y1[i + shift]);
        }

        // Нормирование
        double normFactor = std::sqrt(normY1) * normY2;
        result[shift] = std::abs(sum) / normFactor;
    }

    return result;
}


std::vector<int> OFDM_demod::find_ind_pss(const std::vector<double> corr, float threshold) {
    std::vector<int> indexs;
    for (int i = 0; i < corr.size(); ++i) {
        if (corr[i] > threshold) {
            indexs.push_back(i);
        }
    }
    return indexs;
}


std::vector<cd> OFDM_demod::extract_slots(const std::vector<cd>& signal, const std::vector<int>& indices, int n_slot) {
    // Проверка валидности slot_number
    if (n_slot >= indices.size()) {
        throw std::out_of_range("Неверный номер слота");
    }
    int CPs_len;
    if (CP_LEN == 0) CPs_len = N_FFT / 12.8 + (N_FFT / 12.8 * 0.9)*(OFDM_SYM_IN_SLOT-1);
    else CPs_len = N_FFT / 4 * OFDM_SYM_IN_SLOT;

    int start_index = indices[n_slot] + N_FFT;
    int end_index = start_index + N_FFT * OFDM_SYM_IN_SLOT + CPs_len;
    //std::cout << start_index << " " << end_index << std::endl;

    // Проверка валидности индексов
    if (end_index > signal.size()) {
        throw std::out_of_range("Диапазон превышает размер вектора сигнала");
    }

    // Возврат части сигнала
    return std::vector<cd>(signal.begin() + start_index, signal.begin() + end_index);
}

std::vector<cd> OFDM_demod::extract_symb(const std::vector<cd>& signal, const std::vector<int>& indices, int n_symb) {
    // Проверка валидности slot_number
    if (n_symb >= indices.size()) {
        throw std::out_of_range("Неверный номер слота");
    }
    int cp_len;
    if ((CP_LEN == 0)&&(n_symb == 0)) cp_len = N_FFT / 12.8;
    else if (CP_LEN == 0) cp_len = N_FFT / 12.8 * 0.9;
    else cp_len = N_FFT / 4;

    int start_index = indices[n_symb] + cp_len;
    int end_index = start_index + N_FFT;
    //std::cout << start_index << " " << end_index << std::endl;

    // Проверка валидности индексов
    if (end_index > signal.size()) {
        throw std::out_of_range("Диапазон превышает размер вектора сигнала");
    }

    // Возврат части сигнала
    return std::vector<cd>(signal.begin() + start_index, signal.begin() + end_index);
}


std::vector<double> OFDM_demod::corr_cp_normal(const std::vector<cd>& slot_signal) {
    std::vector<double> corr;//(slot_signal.size(), 0.0);

    int CP_len_b = CP_len;
    for (int i = 0; i < CP_len_b; ++i) {
        std::vector<cd> first_win(slot_signal.begin() + i, slot_signal.begin() + i + CP_len_b);
        std::vector<cd> second_win(slot_signal.begin() + i + N_FFT, slot_signal.begin() + i + N_FFT + CP_len_b);
        std::vector<double> correlat = correlation(first_win, second_win);

        double correlat_cp = 0.0;
        for (auto var : correlat) {
            correlat_cp += var;
        }
        corr.push_back(correlat_cp);
    }

    int CP_len_s = CP_len * 0.9;
    for (int i = CP_len_b; i <= slot_signal.size() - N_FFT; ++i) {
        std::vector<cd> first_win(slot_signal.begin() + i, slot_signal.begin() + i + CP_len_s);
        std::vector<cd> second_win(slot_signal.begin() + i + N_FFT, slot_signal.begin() + i + N_FFT + CP_len_s);
        std::vector<double> correlat = correlation(first_win, second_win);

        double correlat_cp = 0.0;
        for (auto var : correlat) correlat_cp += var;
        corr.push_back(correlat_cp);
    }
    return corr;

}

std::vector<double> OFDM_demod::corr_cp_extended(const std::vector<cd>& slot_signal) {
    std::vector<double> corr(slot_signal.size(), 0.0);

    for (int i = 0; i <= slot_signal.size() - N_FFT - CP_len; ++i) {   
        std::vector<cd> first_win(slot_signal.begin() + i, slot_signal.begin() + i + CP_len);
        std::vector<cd> second_win(slot_signal.begin() + i + N_FFT, slot_signal.begin() + i + N_FFT + CP_len);
        
        std::vector<double> correlat = correlation(first_win, second_win);
        
        double correlat_cp = 0.0;
        for (auto var : correlat) correlat_cp += var;
                
        corr[i] = correlat_cp;
    }

    return corr;
}

std::vector<double> OFDM_demod::corr_cp(const std::vector<cd>& slot_signal) {
    std::vector<double> corr(slot_signal.size(), 0.0);

    if (CP_LEN == 0) corr = corr_cp_normal(slot_signal);
    else             corr = corr_cp_extended(slot_signal);

    return corr;
}


std::vector<int> OFDM_demod::find_ind_cp_normal(const std::vector<double>& corr_cp) {
    std::vector<int> indices;
    int CP_length = N_FFT / 12.8;

    // Первый интервал: от 0 до (N_FFT + CP_LEN) / 2
    int first_end = (N_FFT + CP_length) / 2;
    auto max_it_first = std::max_element(corr_cp.begin(), corr_cp.begin() + first_end);
    int max_index_first = std::distance(corr_cp.begin(), max_it_first);
    indices.push_back(max_index_first);

    CP_length = CP_length * 0.9;

    // Остальные интервалы: с шага (N_FFT + CP_LEN) начиная с first_end
    for (int i = first_end; i < corr_cp.size()-N_FFT-CP_length; i += N_FFT + CP_length) {
        // Определяем конец текущего интервала, не превышая длину corr_cp
        int end = std::min(i + N_FFT + CP_length, static_cast<int>(corr_cp.size()));

        // Находим индекс максимального значения в текущем интервале
        auto max_it = std::max_element(corr_cp.begin() + i, corr_cp.begin() + end);
        int max_index = std::distance(corr_cp.begin(), max_it);

        // Добавляем индекс максимума в результирующий вектор
        indices.push_back(max_index);
    }

    return indices;
}

std::vector<int> OFDM_demod::find_ind_cp_extended(const std::vector<double>& corr_cp) {
    std::vector<int> indices;
    int CP_length = N_FFT / 4;

    // Первый интервал: от 0 до (N_FFT + CP_LEN) / 2
    int first_end = (N_FFT + CP_length) / 2;
    auto max_it_first = std::max_element(corr_cp.begin(), corr_cp.begin() + first_end);
    int max_index_first = std::distance(corr_cp.begin(), max_it_first);
    indices.push_back(max_index_first);

    // Остальные интервалы: с шага (N_FFT + CP_LEN) начиная с first_end
    for (int i = first_end; i < corr_cp.size()-N_FFT-CP_length; i += N_FFT + CP_length) {
        // Определяем конец текущего интервала, не превышая длину corr_cp
        int end = std::min(i + N_FFT + CP_length, static_cast<int>(corr_cp.size()));

        // Находим индекс максимального значения в текущем интервале
        auto max_it = std::max_element(corr_cp.begin() + i, corr_cp.begin() + end);
        int max_index = std::distance(corr_cp.begin(), max_it);

        // Добавляем индекс максимума в результирующий вектор
        indices.push_back(max_index);
    }

    return indices;
}

std::vector<int> OFDM_demod::find_ind_cp(const std::vector<double>& corr_cp) {
    std::vector<int> indices;

    if (CP_LEN == 0) indices = find_ind_cp_normal(corr_cp);
    else             indices = find_ind_cp_extended(corr_cp);

    return indices;
}


std::vector<cd> OFDM_demod::interpolated_H(const std::vector<cd>& signal, int n_slot, int n_symb) {
    std::vector<cd> H_channel(N_PILOTS);
    std::vector<cd> interpolated_signal(N_FFT);
    OFDM_mod ofdm_mod;

    std::vector<int> pilots_ind = ofdm_mod.pilot_indices;
    auto pilot_val = ofdm_mod.getRefs()[n_slot%20][n_symb];

    // Вычисление оценок канала и размещение их
    int k = 0;
    for (int idx : pilots_ind) {
        H_channel[k++] = signal[idx] / pilot_val[k];
        interpolated_signal[idx] = H_channel[k - 1];  // Размещение оценок канала на индексах пилотов
    }

    // Интерполяция между каждой парой контрольных значений в пределах диапазона
    int end_range = N_FFT - G_SUBCAR / 2;
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

    // Продолжает значения после последнего пилота
    int last_pilot = pilots_ind.back();
    cd last_val = interpolated_signal[last_pilot];
    for (int j = last_pilot + 1; j < end_range; ++j) {
        interpolated_signal[j] = last_val;
    }

    // // Output for debug
    // for (auto symb : H_channel) {
    //     std::cout << symb << ",\n";
    // }
    // std::cout << "-------------------\n";

    return interpolated_signal;
}
#include "ofdm_demod.h"
#include "fft/fft.h"

#include "ofdm_mod.h"

#include <iostream>

namespace {
    using cd = std::complex<double>;
}

std::vector<cd> OFDM_demod::convolve(const std::vector<cd>& vec1, const std::vector<cd>& vec2) {
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

    ////////////////////////
    // ДЛЯ облегчения функции можно округление убрать
    ////////////////////////
    // Устанавливаем порог для округления малых значений до нуля
    // const double threshold = 1e-8; // Порог для обнуления
    // for (auto& val : result) {
    //     double real_part = std::real(val); 
    //     double imag_part = std::imag(val);

    //     // Обнуляем реальную и мнимую часть, если она меньше порога
    //     if (std::abs(real_part) < threshold) {
    //         real_part = 0;  
    //     }
    //     if (std::abs(imag_part) < threshold) {
    //         imag_part = 0; 
    //     }

    //     val = cd(real_part, imag_part); // Присваиваем модифицированное комплексное значение обратно
    // }


    return result; // Возвращаем результат корреляции
}


cd OFDM_demod::correlateStatic(const std::vector<cd>& vec1, const std::vector<cd>& vec2, bool norm) {
    // Убедимся, что размеры vec1 и vec2 совпадают
    if (vec1.size() != vec2.size()) {
        throw std::invalid_argument("Vectors vec1 and vec2 must have the same size.");
    }

    // Расчет скалярного произведения
    cd corr = std::inner_product(vec1.begin(), vec1.end(), vec2.begin(), cd(0, 0));

    if (norm) {
        // Нормируем скалярное произведение
        double norm_x = 0;
        double norm_y = 0;

        for (size_t i = 0; i < vec1.size(); ++i) {
            norm_x += std::norm(vec1[i]);        // Считаем норму vec1
            norm_y += std::norm(vec2[i]);        // Считаем норму vec2
        }
        if (norm_x == 0 || norm_y == 0) {
            return cd(0, 0); // Возвращаем комплексное число с нулевыми компонентами
        }
        return corr / std::sqrt(norm_x * norm_y);
    }
    return corr;
}

//Корреляция двух массивов 
//vec1.size() > vec2.size()
std::vector<cd> OFDM_demod::correlateShifted(const std::vector<cd>& vec1, const std::vector<cd>& vec2, bool norm) {
    std::vector<cd> result;
    size_t vec2_size = vec2.size();
    size_t vec1_size = vec1.size();

    // Убедитесь, что vec1 больше, чем vec2
    if (vec1_size < vec2_size) {
        throw std::invalid_argument("vec1 must be larger than vec2");
    }

    // Проходим по всем возможным сдвигам
    for (size_t shift = 0; shift <= vec1_size - vec2_size; ++shift) {
        // Извлекаем часть vec1 длиной vec2_size
        std::vector<cd> sub_vec1(vec1.begin() + shift, vec1.begin() + shift + vec2_size);
        
        // Вызываем функцию корреляции без сдвига
        cd correlation_result = correlateStatic(sub_vec1, vec2, norm);
        
        // Сохраняем результат корреляции
        result.push_back(correlation_result);
    }

    return result;
}

// Обычная и нормализованная корреляция в одной функции
std::vector<double> OFDM_demod::correlate(const std::vector<cd>& vec1, const std::vector<cd>& vec2, bool norm) {
    int n = vec1.size();
    int m = vec2.size();
    std::vector<double> result(n - m + 1, 0.0); // Вектор для возвращения результата

    double max_corr = 0.0; // Для отслеживания максимального значения корреляции

    for (int i = 0; i <= n - m; ++i) {
        cd sum(0, 0);
        for (int j = 0; j < m; ++j) {
            sum += vec1[i + j] * std::conj(vec2[j]); // умножение на комплексно сопряженное
        }
        result[i] = std::abs(sum); // Сохраняем модуль комплексного числа
        if (result[i] > max_corr) {
            max_corr = result[i]; // Обновляем максимальное значение
        }
    }

    // Если norm == true, нормализуем значения в диапазоне от 0 до 1
    if (norm && max_corr > 0.0) {
        for (auto& val : result) {
            val /= max_corr;
        }
    }

    return result;
}

/*
// Возвращает индекс максимального значения в векторе
int maxIndex(const std::vector<cd>& vec) {
    int maxIndex = 0;
    double maxMagnitude = std::abs(vec[0]);

    for (size_t i = 1; i < vec.size(); ++i) {
        double magnitude = std::abs(vec[i]);
        if (magnitude > maxMagnitude) {
            maxMagnitude = magnitude;
            maxIndex = i;
        }
    }

    return maxIndex;
}

// Возвращает первый индекс который больше порогового значения
// threshold ≈ 0.99
int findFIndexThreshold(const std::vector<cd>& vec, double threshold) {
    for (size_t i = 0; i < vec.size(); ++i) {
        if (std::abs(vec[i]) > threshold) {
            return i; // Возвращаем индекс первого найденного значения
        }
    }
    return -1; // Если не найдено, возвращаем -1
}

std::vector<int> indexsPSS (const std::vector<cd>& vec) {
    OFDM_mod ofdm_mod;
    std::vector<cd> timePSS = ofdm_mod.mapPSS();

    std::vector<cd> corr = correlateShifted(vec, timePSS, true);

    int first_index = findFIndexThreshold(corr);
    
}
*/
// Функция корреляции для нахождения циклического префикса
std::vector<cd> OFDM_demod::CP_CorrIndexs(const std::vector<cd>& vec) {
    const short int n = vec.size();
    std::vector<cd> correlations(n, 0.0);
    
    // Корреляция для каждой позиции
    for (int i = 0; i < n; ++i) {
        std::vector<cd> cp_window(vec.begin() + i, vec.begin() + i + CP_LEN);
        std::vector<cd> next_window(vec.begin() + i + N_FFT, vec.begin() + i + N_FFT + CP_LEN);

        correlations[i] = correlateStatic(cp_window, next_window, true);
    }

    return correlations;
}

/*
// Функция для поиска индекса максимальной корреляции (индекс циклического префикса)
int findMaxCorrelationIndex(const std::vector<std::complex<double>>& data) {
    std::vector<double> correlations = findCyclicPrefixCorrelation(data);
    
    // Поиск индекса максимального значения
    int max_index = 0;
    for (int i = 1; i < correlations.size(); ++i) {
        if (correlations[i] > correlations[max_index]) {
            max_index = i;
        }
    }

    return max_index;
}
*/

// Функция для вычисления взаимной корреляции между двумя векторами
std::vector<double> correlation(const std::vector<std::complex<double>>& y1, const std::vector<std::complex<double>>& y2) {
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


std::vector<int> find_indexs_pss(std::vector<double> corr, float threshold) {
    std::vector<int> indexs;
    for (int i = 0; i < corr.size(); ++i) {
        if (corr[i] > threshold) {
            indexs.push_back(i);
        }
    }
    return indexs;
}

std::vector<cd> extract_slots(const std::vector<cd>& signal, const std::vector<int>& indices, int slot_number) {
    // Проверка валидности slot_number
    if (slot_number >= indices.size()) {
        throw std::out_of_range("Неверный номер слота");
    }

    int start_index = indices[slot_number] + N_FFT;
    int end_index = start_index + (N_FFT + CP_LEN) * OFDM_SYM_IN_SLOT;
    //std::cout << start_index << " " << end_index << std::endl;

    // Проверка валидности индексов
    if (end_index > signal.size()) {
        throw std::out_of_range("Диапазон превышает размер вектора сигнала");
    }

    // Возврат части сигнала
    return std::vector<cd>(signal.begin() + start_index, signal.begin() + end_index);
}

std::vector<double> corr_cp(const std::vector<cd>& slot_signal) {
    std::vector<double> corr(slot_signal.size(), 0.0);
    std::vector<int> indices;


    for (int i = 0; i <= slot_signal.size() - N_FFT - CP_LEN; ++i) {   
        std::vector<cd> first_win(slot_signal.begin() + i, slot_signal.begin() + i + CP_LEN);
        std::vector<cd> second_win(slot_signal.begin() + i + N_FFT, slot_signal.begin() + i + N_FFT + CP_LEN);
        
        // Вычисление корреляции между окнами
        OFDM_demod ofdm_demod;
        std::vector<double> correlat = correlation(first_win, second_win);
        
        double correlat_cp = 0.0;
        for (auto var : correlat) {
            correlat_cp += var;
        }
                
        corr[i] = correlat_cp;
    }

    return corr;
}

std::vector<int> find_max_cp(const std::vector<double>& corr_cp) {
    std::vector<int> indices;

    // Первый интервал: от 0 до (N_FFT + CP_LEN) / 2
    int first_end = (N_FFT + CP_LEN) / 2;
    auto max_it_first = std::max_element(corr_cp.begin(), corr_cp.begin() + first_end);
    int max_index_first = std::distance(corr_cp.begin(), max_it_first);
    indices.push_back(max_index_first);

    // Остальные интервалы: с шага (N_FFT + CP_LEN) начиная с first_end
    for (int i = first_end; i < corr_cp.size()-N_FFT-CP_LEN; i += N_FFT + CP_LEN) {
        // Определяем конец текущего интервала, не превышая длину corr_cp
        int end = std::min(i + N_FFT + CP_LEN, static_cast<int>(corr_cp.size()));

        // Находим индекс максимального значения в текущем интервале
        auto max_it = std::max_element(corr_cp.begin() + i, corr_cp.begin() + end);
        int max_index = std::distance(corr_cp.begin(), max_it);

        // Добавляем индекс максимума в результирующий вектор
        indices.push_back(max_index);
    }

    return indices;
}
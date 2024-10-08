#include "ofdm_demod.h"
#include "fft/fft.h"
#include <cmath>
#include <complex>
#include <algorithm>

#include <iostream>

namespace {
    using cd = std::complex<double>;
}

std::vector<cd> OFDM_demod::fft_convolve(const std::vector<cd>& vec1, const std::vector<cd>& vec2) {
    // Вычисляем размер результирующего вектора свёртки
    size_t n = vec1.size() + vec2.size() - 1;
    
    // Определяем размер FFT как ближайшую степень двойки
    size_t fft_size = 1;
    while (fft_size < n) {
        fft_size *= 2;
    }
    
    // Создаём векторы с нулями для выполнения FFT
    std::vector<cd> a(fft_size, cd(0, 0));
    std::vector<cd> b(fft_size, cd(0, 0));

    // Копируем входные векторы в нулевые векторы
    a.assign(vec1.begin(), vec1.end());
    b.assign(vec2.begin(), vec2.end());

    // Выполняем FFT для обоих векторов
    a = fft(a);
    b = fft(b);

    // Умножаем векторы в частотной области
    std::vector<cd> result(fft_size);
    for (size_t i = 0; i < fft_size; ++i) {
        result[i] = a[i] * b[i];
    }

    // Выполняем обратное FFT на результате
    result = ifft(result);

    // Устанавливаем порог для округления малых значений до нуля
    const double threshold = 0.00001;
    for (auto& val : result) {
        if (std::abs(val) < threshold) {
            val = cd(0, 0);  // Обнуляем комплексное значение
        }
    }

    // Изменяем размер результата до нужной длины свёртки
    result.resize(n);

    return result;
}

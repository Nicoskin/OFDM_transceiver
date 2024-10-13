#include "../ofdm_demod.h"
#include "../fft/fft.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>

// g++ demod_fft_conv_time.cpp ../ofdm_demod.cpp ../fft/fft.cpp -o test && ./test

int main() {
    OFDM_demod OFDM_demod;

    // Создание массива из 1 миллиона значений
    std::vector<std::complex<double>> large_vec(1e6);
    for (size_t i = 0; i < large_vec.size(); ++i) {
        large_vec[i] = {static_cast<double>(i % 100), static_cast<double>(i % 100)};
    }

    // Создание массива из 10 значений
    std::vector<std::complex<double>> small_vec = {
        {1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5},
        {6, 6}, {7, 7}, {8, 8}, {9, 9}, {10, 10}
    };

    // Тестирование функции convolve
    auto start_convolve = std::chrono::high_resolution_clock::now();
    auto convolved = OFDM_demod.convolve(large_vec, small_vec);
    auto end_convolve = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration_convolve = end_convolve - start_convolve;

    // Тестирование функции correlate_with_shift
    auto start_correlate = std::chrono::high_resolution_clock::now();
    auto corr = OFDM_demod.correlateShifted(large_vec, small_vec, true);
    auto end_correlate = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration_correlate = end_correlate - start_correlate;

    // Печать результатов
    std::cout << "Convolution took: " << duration_convolve.count() << " seconds." << std::endl;
    std::cout << "Correlation took: " << duration_correlate.count() << " seconds." << std::endl;

    return 0;
}


// Проведённые тесты:
// 1млн
// Convolution took: 1.83882 seconds.
// Correlation took: 0.743936 seconds.

// 10млн
// Convolution took: 33.6569 seconds.
// Correlation took: 8.63476 seconds.

// 4 194 304 (2^22)
// Convolution took: 16.6048 seconds.
// Correlation took: 4.37574 seconds.
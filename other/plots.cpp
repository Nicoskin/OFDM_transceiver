#include <iostream>
#include <vector>
#include <complex>
#include <algorithm>
#include "plots.h" 

namespace plt = matplotlibcpp;
using cd = std::complex<double>;


// Явное инстанцирование шаблона для unsigned char
template void cool_plot<unsigned char>(const std::vector<unsigned char>&, std::string, std::string, bool);

// Явная инстанциация для типа double
template void cool_plot<double>(const std::vector<double>&, std::string, std::string, bool);

// Первая версия функции: для одного вектора комплексных чисел
void cool_plot(const std::vector<cd>& data, std::string vid, std::string title, bool show_plot) {

    std::vector<double> real_part, imag_part;
    for (size_t i = 0; i < data.size(); ++i) {
        real_part.push_back(std::real(data[i]));
        imag_part.push_back(std::imag(data[i]));
    }

    double max_real = *std::max_element(real_part.begin(), real_part.end());
    double max_imag = *std::max_element(imag_part.begin(), imag_part.end());
    double min_real = *std::min_element(real_part.begin(), real_part.end());
    double min_imag = *std::min_element(imag_part.begin(), imag_part.end());

    double maximum = std::max(max_real, max_imag);
    double minimum = std::min(min_real, min_imag);

    plt::figure_size(1100, 600);
    plt::subplots_adjust({{"left", 0.07}, {"bottom", 0.08}, {"right", 0.95}, {"top", 0.92}});

    plt::title(title);
    plt::plot(real_part, vid);
    plt::plot(imag_part, vid);
    plt::grid(true);

    plt::xlim(-(data.size() * 0.02), data.size() * 1.02);
    plt::ylim(-(maximum * 1.15), maximum * 1.15);

    if (show_plot) {
        plt::show();
        plt::detail::_interpreter::kill();
    }
}

// Вторая версия функции: для одного вектора чисел (double)
template <typename T>
void cool_plot(const std::vector<T>& input, std::string vid, std::string title, bool show_plot) {
    std::vector<double> y;
    y.reserve(input.size());  // Зарезервировать память для нового вектора

    for (const T& value : input) {
        y.push_back(static_cast<double>(value));  // Преобразуем в double и добавляем в новый вектор
    }

    std::vector<double> x(y.size());
    for (size_t i = 0; i < y.size(); ++i) {
        x[i] = static_cast<double>(i);  // Используем индекс как ось X
    }

    double maximum = *std::max_element(y.begin(), y.end());
    double minimum = *std::min_element(y.begin(), y.end());

    plt::figure_size(1100, 600);
    plt::subplots_adjust({{"left", 0.07}, {"bottom", 0.08}, {"right", 0.95}, {"top", 0.92}});

    plt::title(title);
    plt::plot(x, y, vid);
    plt::grid(true);

    if (minimum >= 0) {
        plt::xlim(-(x.size() * 0.02), x.size() * 1.02);
        plt::ylim(-(maximum * 0.05), maximum * 1.05);
    } else {
        plt::xlim(-(x.size() * 0.02), x.size() * 1.02);
        plt::ylim(-(maximum * 1.05), maximum * 1.05);
    }

    if (show_plot) {
        plt::show();
        plt::detail::_interpreter::kill();
    }
}

void cool_scatter(const std::vector<cd>& data, const std::string title, bool show_plot) {
    // Prepare data for x and y
    std::vector<double> real_part, imag_part, maxi, colors;
    real_part.reserve(data.size());
    imag_part.reserve(data.size());
    int k = 0;
    for (const auto& val : data) {
        real_part.push_back(std::real(val)); // Real part for x
        imag_part.push_back(std::imag(val)); // Imaginary part for y
        maxi.push_back(std::abs(val));
        colors.push_back(static_cast<double>(k++) / data.size());
    }

    double maximum = *std::max_element(maxi.begin(), maxi.end());


    // Configure the scatter plot
    plt::figure_size(700, 700);
    plt::subplots_adjust({{"left", 0.07}, {"bottom", 0.05}, {"right", 0.94}, {"top", 0.94}});
    plt::title(title);
    plt::grid(true);
    plt::xlim(-(1.05 * maximum), 1.05 * maximum);
    plt::ylim(-(1.05 * maximum), 1.05 * maximum);
    plt::scatter_colored(real_part, imag_part, colors, 10.0, "hsv");

    // Add axis lines
    plt::axhline(0, 0, 1, {{"color", "black"}, {"linestyle", "--"}, {"linewidth", "1"}});
    plt::axvline(0, 0, 1, {{"color", "black"}, {"linestyle", "--"}, {"linewidth", "1"}});


    if (show_plot) {
        plt::show();
        plt::detail::_interpreter::kill();
    } 
}

void show_plot() {
    plt::show();
    plt::detail::_interpreter::kill();
}


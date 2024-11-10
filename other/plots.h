#ifndef COOL_PLOT_H
#define COOL_PLOT_H

#include <vector>
#include <complex>
#include <string>
#include <numeric>

#include "../matplotlibcpp.h"

namespace plt = matplotlibcpp;
using cd = std::complex<double>;

// Функция для отображения графика с комплексными числами
void cool_plot(const std::vector<cd>& data, 
               std::string vid = "-", 
               std::string title = "", 
               bool show_plot = false);

// Функция для отображения графика с одним вектором чисел (y)
template <typename T>
void cool_plot(const std::vector<T>& input, 
               std::string vid = "-", 
               std::string title = "", 
               bool show_plot = false);


void cool_scatter(const std::vector<cd>& x, const std::string title = "", bool show_plot = false);

void show_plot();

#endif  // COOL_PLOT_H
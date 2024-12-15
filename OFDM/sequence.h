#ifndef SEQUENSE_H
#define SEQUENSE_H

#include <vector>
#include <complex>
#include <cmath>
#include "../config.h"

using cd = std::complex<double>;

void gen_pilots_siq(std::vector<std::vector<std::vector<cd>>>& refs, int N_cell = N_CELL_ID, bool amp_pilots_high = true);
std::vector<cd> ZadoffChu(int u = 25);
std::vector<cd> generate_sss(int N_ID_cell, int subframe = 0, bool add_zero_middle = false);

#endif // SEQUENSE_H
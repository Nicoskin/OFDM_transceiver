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

using cd = std::complex<double>;

std::vector<std::complex<double>> readComplexNumsFromFile(const std::string& filename);
std::vector<int> calculate_pci(std::vector<cd>& complexVector);
std::vector<cd> interpolated_channel_estimator(const std::vector<cd> signal, int n_slot, int n_symb, std::vector<std::vector<std::vector<cd>>> refs, bool zero_sym);
std::vector<cd> interpolating_H_0to4_symb(std::vector<cd> H_0, std::vector<cd> H_4);
std::vector<cd> time_pbch_without_cp(std::vector<cd> complexVector, int index_first_pss);
#include <iostream>
#include <vector>
#include <cstdint>

std::vector<uint8_t> scrambling(std::vector<uint8_t>bits, int N_cell);
// std::vector<uint8_t> pbch_de_rate_matching(const std::vector<uint8_t>& input_bits);
// std::vector<uint8_t> viterbi_decode(const std::vector<uint8_t>& encoded_bits);
std::vector<int> rate_dematching(const std::vector<int>& received, int D);
std::vector<int> rate_match(const std::vector<std::vector<int>> &d, int out_len);
std::vector<int> _rate_match(const std::vector<std::vector<int>> &d, int E);

std::vector<int> de_rate_match(const std::vector<int> &e, int D);
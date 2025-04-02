#include <iostream>
#include <vector>
#include <cstdint>

std::vector<uint8_t> scrambling(std::vector<uint8_t>bits, int N_cell, int frame);

std::vector<int> rate_match(const std::vector<int> &d, int out_len);
std::vector<int> _rate_match(const std::vector<int> &d, int E);

std::vector<uint8_t> de_rate_match(const std::vector<uint8_t>& e, int D);

bool check_crc(const std::vector<uint8_t> &c);
std::vector<uint8_t> calculate_crc_16(const std::vector<uint8_t>& bitData);

std::vector<int> encode(const std::vector<int>& input);
std::vector<uint8_t> decode(const std::vector<uint8_t>& input);
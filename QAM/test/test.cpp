﻿#include <iostream>
#include <vector>
#include "../qam_mod.h"

// g++ test.cpp ../qam_mod.cpp -o test && ./test

int main() {
    std::vector<std::vector<uint8_t>> bits = {{0, 1, 1, 0, 1, 1, 0, 0, 0, 0}}; 

    auto bpsk_mod = modulate(bits, 1);
    auto qpsk_mod = modulate(bits, 2);
    auto qam16_mod = modulate(bits, 4);
    auto qam64_mod = modulate(bits, 6);

    std::cout << "BPSK Modulation:" << std::endl;
    for (const auto& symbol_vec : bpsk_mod) {
        for (const auto& symbol : symbol_vec) {
            std::cout << "(" << symbol.real() << ", " << symbol.imag()  << "),  "; //<< std::endl;
        }
        std::cout << std::endl;
    }

    std::cout << "\nQPSK Modulation:" << std::endl;
    for (const auto& symbol_vec : qpsk_mod) {
        for (const auto& symbol : symbol_vec) {
            std::cout << "(" << symbol.real() << ", " << symbol.imag()  << "),  "; //<< std::endl;
        }
        std::cout << std::endl;
    }

    std::cout << "\n16-QAM Modulation:" << std::endl;
    for (const auto& symbol_vec : qam16_mod) {
        for (const auto& symbol : symbol_vec) {
            std::cout << "(" << symbol.real() << ", " << symbol.imag()  << "),  "; //<< std::endl;
        }
        std::cout << std::endl;
    }

    std::cout << "\n64-QAM Modulation:" << std::endl;
    for (const auto& symbol_vec : qam64_mod) {
        for (const auto& symbol : symbol_vec) {
            std::cout << "(" << symbol.real() << ", " << symbol.imag()  << "),  "; //<< std::endl;
        }
        std::cout << std::endl;
    }


    return 0;
}

#include <iostream>
#include <iomanip>
#include "../Segmenter/segmenter.h"
#include "../QAM_Modulation/modulation.h"

void print_modulation(const Modulation* mod) {
    if (mod) {
        for (const auto& symbol : mod->symbols) {
            std::cout << "(" << std::real(symbol) << " " << std::showpos << std::imag(symbol) << "i)" << std::endl;
        }
    }
}

int main() {
    // Входные биты
    std::vector<uint8_t> bits = {
    0,1,1,0,1,0,0,1, 1,0,0,1,0,1,1,0, 1,0,0,1,0,1,1,0,
    1,0,0,1,0,1,1,0, 1,0,0,1,0,1,1,0,
    0,1,1,0,1,0,0,1, 1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,}; 

//
// Сегментер
//
    Segmenter segmenter;
    auto segments = segmenter.segment(bits);
    /*
    Сегмент 1:
    00000000 00000000 00000000 00000000 00000000 01000000 01101001 10010110 10010110 10010110 10010110 01101001 11111111 11111111 00000000 11110111

    Сегмент 2:
    00000000 00000001 00000000 00000000 00000000 00001000 11111111 10100110 01000101 01000001 01101010 11101111 01010111 00010111 01110010 00100111
    */

   // Вывод сегментов - биты
   for (const auto& segment : segments) {
       for (const auto& bit : segment) {
           std::cout << static_cast<int>(bit);
       }
       std::cout << std::endl;
   }

//
// Модуляция
//

    auto qpsk_mod = modulate(segments, QPSK);
    
    // Вывод сегментов - QPSK
    for (const auto& symbol_vec : qpsk_mod) {
        for (const auto& symbol : symbol_vec) {
            std::cout << "(" << std::setprecision(3) << symbol.real() << ", " << symbol.imag()  << "),  "; //<< std::endl;
        }
        std::cout << std::endl;
    }

    return 0;
}

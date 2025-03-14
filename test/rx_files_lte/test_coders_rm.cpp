#include <iostream>
#include "coders.h"
#include "pbch.h"

int main(){
    std::vector<int> input = {0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,1,0,1,0,1,1,0,0,1,1,0,1,0};
        std::cout << "Input: (" << input.size() << ")\n";
        for (int bit : input) std::cout << (bit & 1) << "";
        std::cout << "\n";
    
    ConvolutionalEncoder encoder;
    auto encod = encoder.encode(input); 
        std::cout << "Encoded: (" << encod.size() * encod[0].size() << ")\n";
        for (size_t g = 0; g < encod.size(); ++g) {
            for (size_t i = 0; i < encod[0].size(); ++i) {
                std::cout << encod[g][i];
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
        encoder.print_encoded(encod);

    auto rm_tx = _rate_match(encod, 480);
        std::cout << "Rm_tx: (" << rm_tx.size() << ")\n";
        for (int bit : rm_tx) std::cout << bit << "";
        std::cout << "\n";

    auto rm_rx = de_rate_match(rm_tx, 120);
        std::cout << "Rm_rx: (" << rm_rx.size() << ")\n";
        for (int bit : rm_rx) std::cout << (bit & 1) << "";
        std::cout << "\n";

    ViterbiDecoder decoder;
    auto decod = decoder.decode(rm_rx);
        std::cout << "Decoded: (" << decod.size() << ")\n";
        for (int bit : decod) std::cout << (bit & 1) << "";
        std::cout << "\n";


    return 0;
}
#include "../ofdm_mod.h"
#include "../fft/fft.h"
#include "../sequence.h"
#include <iostream>

// g++ test_rs.cpp ../ofdm_mod.cpp ../fft/fft.cpp ../sequence.cpp -o test_rs && ./test_rs

int main(){
 
    int pci = 31;
    std::vector<std::vector<std::vector<cd>>> refs;
    refs.resize(20, std::vector<std::vector<cd>>(7, std::vector<cd>(6 * 2, cd{0, 0})));
    gen_pilots_siq(refs, pci, true);    
    for(size_t ns = 0; ns < 2; ns++){
        for(size_t l = 0; l < 7; l++){
            std::cout << "ns: "<< ns << "  l: " << l << std::endl;
            for(size_t m = 0; m < 6 * 2; m++){
                std::cout << refs[ns][l][m] << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

    return 0;
}
#include "coders.h"
#include <cmath>
#include <iostream>

//
// ConvolutionalEncoder
//
std::vector<std::vector<int>> ConvolutionalEncoder::encode(const std::vector<int>& input) {
    std::vector<std::vector<int>> output(GENERATORS.size(), std::vector<int>(input.size(), 0));
    std::vector<int> shift_register(CONSTRAINT_LENGTH, 0);
    
    for (size_t i = 0; i < input.size(); ++i) {

        std::copy_backward(shift_register.begin(), shift_register.end() - 1, shift_register.end());
        shift_register[0] = (i < input.size()) ? input[i] : 0;
        
        for (size_t g = 0; g < GENERATORS.size(); ++g) {
            int encoded_bit = 0;
            for (size_t j = 0; j < CONSTRAINT_LENGTH; ++j) {
                if ((GENERATORS[g] >> CONSTRAINT_LENGTH - j - 1 ) & 1) { 
                    encoded_bit ^= shift_register[j];
                }
            }
            output[g][i] = encoded_bit;
        }
    }
    return output;
}

void ConvolutionalEncoder::print_encoded(const std::vector<std::vector<int>>& encoded) {
    std::cout << "Encoded: (" << encoded.size() * encoded[0].size() << ")\n";
    for (size_t i = 0; i < encoded[0].size(); ++i) {
        for (size_t g = 0; g < encoded.size(); ++g) {
            std::cout << encoded[g][i];
        }
    }
    std::cout << std::endl;
}

//
// ViterbiDecoder
//
int ViterbiDecoder::hammingDistance(int a, int b) {
    int diff = a ^ b, count = 0;
    while (diff) {
        count += diff & 1;
        diff >>= 1;
    }
    return count;
}

int ViterbiDecoder::encodeState(int state, int inputBit) {
    int output = 0;
    int newState = ((state << 1) | inputBit) & (NUM_STATES - 1);
    for (size_t g = 0; g < GENERATORS.size(); ++g) {
        int encoded_bit = 0;
        for (size_t j = 0; j < CONSTRAINT_LENGTH; ++j) {
            if ((GENERATORS[g] >> (CONSTRAINT_LENGTH - j - 1)) & 1) {
                encoded_bit ^= ((newState >> j) & 1);
            }
        }
        output |= (encoded_bit << g);
    }
    return output;
}

std::vector<int> ViterbiDecoder::decode(const std::vector<int>& received) {
    int numSteps = received.size() / GENERATORS.size();
    std::vector<std::vector<int>> pathMetric(numSteps + 1, std::vector<int>(NUM_STATES, INF));
    std::vector<std::vector<int>> prevState(numSteps + 1, std::vector<int>(NUM_STATES, -1));
    std::vector<std::vector<int>> prevBit(numSteps + 1, std::vector<int>(NUM_STATES, -1));

    pathMetric[0][0] = 0;

    for (int step = 0; step < numSteps; ++step) {
        for (int state = 0; state < NUM_STATES; ++state) {
            if (pathMetric[step][state] == INF) continue;
            for (int bit = 0; bit < 2; ++bit) {
                int nextState = ((state << 1) | bit) & (NUM_STATES - 1);
                int expected = encodeState(state, bit);
                int receivedBits = 0;
                for (size_t g = 0; g < GENERATORS.size(); ++g) {
                    receivedBits |= (received[step * GENERATORS.size() + g] << g);
                }
                int cost = hammingDistance(receivedBits, expected);
                int newMetric = pathMetric[step][state] + cost;
                if (newMetric < pathMetric[step + 1][nextState]) {
                    pathMetric[step + 1][nextState] = newMetric;
                    prevState[step + 1][nextState] = state;
                    prevBit[step + 1][nextState] = bit;
                }
            }
        }
    }

    int bestState = 0;
    for (int state = 1; state < NUM_STATES; ++state) {
        if (pathMetric[numSteps][state] < pathMetric[numSteps][bestState]) {
            bestState = state;
        }
    }

    std::vector<int> decodedBits(numSteps);
    for (int step = numSteps; step > 0; --step) {
        decodedBits[step - 1] = prevBit[step][bestState];
        bestState = prevState[step][bestState];
    }
    return decodedBits;
}

    


// int main() {
//     ConvolutionalEncoder encoder;
//     ViterbiDecoder decoder;
    
//     std::vector<int> input = {0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0};
    
//         std::cout << "Input: \n";
//         for (int bit : input) std::cout << (bit & 1) << "";
//         std::cout << "\n";

//     auto encoded = encoder.encode(input);
    
//     std::vector<int> encoded_flat;
//     std::cout << "Encoded: \n";
//     for (size_t i = 0; i < encoded[0].size(); ++i) {
//         for (size_t g = 0; g < encoded.size(); ++g) {
//             encoded_flat.push_back(encoded[g][i]);
//             std::cout << encoded[g][i];
//         }
//     }
//     std::cout << "\n";

//     auto decoded = decoder.decode(encoded_flat);
    
//     std::cout << "Decoded: \n";
//     for (int bit : decoded) std::cout << (bit & 1) << "";
//     std::cout << "\n";

//     return 0;
// }
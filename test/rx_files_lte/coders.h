#ifndef CODERS_H
#define CODERS_H

#include <vector>
#include <limits>

class ViterbiDecoder {
private:
    const std::vector<int> GENERATORS = {0133, 0171, 0165};
    const int CONSTRAINT_LENGTH = 7;
    const int NUM_STATES = 1 << (CONSTRAINT_LENGTH - 1);
    const int INF = std::numeric_limits<int>::max() / 2;

    int hammingDistance(int a, int b);
    int encodeState(int state, int inputBit);

public:
    std::vector<int> decode(const std::vector<int>& received);
};


class ConvolutionalEncoder {
private:
    
    const std::vector<int> GENERATORS = {0133, 0171, 0165};
    const int CONSTRAINT_LENGTH = 7;
    
public:
    std::vector<std::vector<int>> encode(const std::vector<int>& input);
    
    void print_encoded(const std::vector<std::vector<int>>& encoded);
};

#endif // CODERS_H
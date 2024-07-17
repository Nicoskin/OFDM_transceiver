#include "fft.h"
#include <algorithm>
#include <numbers>

namespace
{
    using cd = std::complex<double>;
    const double PI = std::numbers::pi;
}

int reverse(int num, int lg_n) {
    int res = 0;
    for (int i = 0; i < lg_n; i++) {
        if (num & (1 << i))
            res |= 1 << (lg_n - 1 - i);
    }
    return res;
}

std::vector<cd> fft(const std::vector<cd> &num) {
    std::vector<cd> fft_image{num};
    int n = num.size();
    int lg_n = 0;
    while ((1 << lg_n) < n)
        lg_n++;

    for (int i = 0; i < n; i++) {
        if (i < reverse(i, lg_n))
            std::swap(fft_image[i], fft_image[reverse(i, lg_n)]);
    }

    for (int len = 2; len <= n; len <<= 1) {
        double ang = 2 * PI / len * -1;
        cd wlen = std::polar(1.0, ang);
        for (int i = 0; i < n; i += len) {
            cd w(1);
            for (int j = 0; j < len / 2; j++) {
                cd u = fft_image[i + j], v = fft_image[i + j + len / 2] * w;
                fft_image[i + j] = u + v;
                fft_image[i + j + len / 2] = u - v;
                w *= wlen;
            }
        }
    }

    return fft_image;
}

std::vector<cd> ifft(const std::vector<cd> &num) {
    std::vector<cd> ifft_image = fft(num);
    int n = ifft_image.size();

    for (cd & x : ifft_image)
        x /= n;

    std::reverse(ifft_image.begin() + 1, ifft_image.end());
    return ifft_image;
}

std::vector<cd> fftshift(const std::vector<cd> &num) {
    std::vector<cd> shifted(num.size());
    int n = num.size();
    int mid = (n + 1) / 2;

    for (int i = 0; i < n; i++) {
        shifted[i] = num[(i + mid) % n];
    }

    return shifted;
}

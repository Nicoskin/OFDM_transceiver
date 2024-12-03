#ifndef CHAT_H
#define CHAT_H

    #include <fstream>
    #include <vector>
    #include <complex>
    #include <string>
    #include <cmath>
    #include <limits>
    #include <unistd.h> // Для функции sleep
    #include <omp.h>
    #include <thread> // Для многозадачности
    #include <atomic> // Для безопасного взаимодействия между потоками
    #include <mutex>

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
    #include "chat.h"

    #include <iostream>
    #include <sys/ioctl.h> // Для определения размера терминала

    void sdrSettings();
    void viewTransmissionSettings();
    void clearScreen(); // ANSI escape sequence для очистки экрана
    void chatMode();
    

#endif
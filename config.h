#ifndef CONFIG_H
#define CONFIG_H

// Параметры OFDM
#define N_FFT 128
#define G_SUBCAR 55
#define N_PILOTS 6
#define CP_LEN 16

#define IQ_MODULATION 2 // BPSK 1, QPSK 2, 16QAM 4, 64QAM 6
#define OFDM_SYM_IN_SLOT 5

// Параметры сегментера 
#define MAX_LEN_LINE ((N_FFT-G_SUBCAR-N_PILOTS-1) * IQ_MODULATION * OFDM_SYM_IN_SLOT) // активных 330 поднесущих, с данными 272
#define SEGMENT_NUM_BITS 10
#define USEFUL_BITS 32
#define CRC_BITS 16



#endif //CONFIG_H
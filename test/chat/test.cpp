
#include "chat.h"



using cd = std::complex<double>;
namespace plt = matplotlibcpp;


// g++ test.cpp -I/usr/include/python3.10 -lpython3.10 -fopenmp ../../other/plots.cpp ../../other/model_channel.cpp ../../File_converter/file_converter.cpp  ../../QAM/qam_mod.cpp ../../QAM/qam_demod.cpp ../../Segmenter/segmenter.cpp ../../OFDM/ofdm_mod.cpp ../../OFDM/ofdm_demod.cpp ../../OFDM/fft/fft.cpp ../../OFDM/sequence.cpp ../../OFDM/freq_offset.cpp -o test && ./test





// void mainMenu() {
//     while (true) {
//         clearScreen();
//         std::cout << "=== Меню программы ===\n"
//                   << "1 - Настройка своей SDR\n"
//                   << "2 - Посмотреть настройки передачи\n"
//                   << "3 - Перейти в чат\n"
//                   << "q - Выйти\n"
//                   << "======================\n";

//         std::cout << "↪ ";
//         char choice;
//         std::cin >> choice;
//         std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

//         if (choice == '1') {
//             sdrSettings();
//         } else if (choice == '2') {
//             viewTransmissionSettings();
//         } else if (choice == '3') {
//             chatMode();
//         } else if (choice == 'q') {
//             std::cout << "Выход из программы...\n";
//             break;
//         } else {
//             std::cout << "Неверная команда. Попробуйте снова.\n";
//             sleep(1);
//         }
//     }
// }

// int main() {
//     mainMenu();
//     return 0;
// }
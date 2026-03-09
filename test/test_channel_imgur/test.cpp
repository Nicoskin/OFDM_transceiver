#include <iostream>
#include <fstream>
#include <vector>
#include <complex>
#include <string>
#include <cmath>

#include <omp.h>


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
// #include "../../other/plots.h"

#include <GL/glew.h>
#include <SDL2/SDL.h>

#include "imgui.h"
#include "implot.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_sdl2.h"



using cd = std::complex<double>;
// namespace plt = matplotlibcpp;


// g++ test.cpp -I/usr/include/python3.10 -lpython3.10 -fopenmp ../../other/plots.cpp ../../other/model_channel.cpp ../../File_converter/file_converter.cpp  ../../QAM/qam_mod.cpp ../../QAM/qam_demod.cpp ../../Segmenter/segmenter.cpp ../../OFDM/ofdm_mod.cpp ../../OFDM/ofdm_demod.cpp ../../OFDM/fft/fft.cpp ../../OFDM/sequence.cpp ../../OFDM/freq_offset.cpp -o test && ./test

void saveD(const std::vector<double>& arr, const std::string& filename) {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cerr << "Не удалось открыть файл для записи." << std::endl;
        return;
    }

    for (const auto& value : arr) {
        outFile << value << "\n";
    }
}

void saveCD(const std::vector<cd>& arr, const std::string& filename) {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cerr << "Не удалось открыть файл для записи." << std::endl;
        return;
    }

    for (const auto& value : arr) {
        outFile << value << "\n";
    }
}

std::vector<double> corr_cp_extended(const std::vector<cd>& slot_signal) {
    std::vector<double> corr(slot_signal.size(), 0.0);
    int CP_len = 9;
    OFDM_demod ofdm_demod;

    for (int i = 0; i <= slot_signal.size() - N_FFT - CP_len; ++i) {   
        std::vector<cd> first_win(slot_signal.begin() + i, slot_signal.begin() + i + CP_len);
        std::vector<cd> second_win(slot_signal.begin() + i + N_FFT, slot_signal.begin() + i + N_FFT + CP_len);
        
        std::vector<double> correlat = ofdm_demod.correlation(first_win, second_win);
        
        double correlat_cp = 0.0;
        for (auto var : correlat) correlat_cp += std::abs(var);
                
        corr[i] = correlat_cp;
    }

    return corr;
}

std::vector<cd> transmission(){
    std::cout << "-----TX-----" << std::endl;

    auto bits = generateRandBits(680*2, 1);
    // auto bits = string2bits("Hello, World! Привет, Мир! 1234567890");
    //auto bits = file2bits("test_file_in/арбуз арбуз.jpeg");
    
    Segmenter segmenter;
    auto segments = segmenter.segment(bits, 0); // Flag: 0 случайные биты, 1 - текст, 2 - файл
    segmenter.get_size_data_in_slot();
    segments = segmenter.scramble(segments);

    QAM_mod qam_mod;
    auto qpsk_mod = qam_mod.modulate(segments);

    OFDM_mod ofdm_mod = OFDM_mod(false);
    auto ofdm_data = ofdm_mod.modulate(qpsk_mod);

    return ofdm_data;
}

std::vector<cd> add_ch(std::vector<cd>& tx_signal) {
    double SNR_dB = 20.0;
    auto signal = pad_zeros(tx_signal, 1020, 1000);
    signal = add_CFO(signal, 2000);
    signal = add_Channel(signal, {1.0, 0.4});
    auto noise_signal = add_noise(signal, SNR_dB, 1);

    return noise_signal;
}

std::vector<std::vector<uint8_t>> receive(std::vector<cd>& noise_signal) {
    std::cout << "-----RX-----" << std::endl;

        auto start = std::chrono::high_resolution_clock::now();
    OFDM_mod ofdm_mod;
    std::vector<std::complex<double>> noise_signal_cfo;
    frequency_correlation(ofdm_mod.mapPSS(), noise_signal, 15000, noise_signal_cfo, 1920000);

    OFDM_demod ofdm_demod = OFDM_demod(false);
    auto demod_signal = ofdm_demod.demodulate(noise_signal_cfo);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Time for OFDM demodulation: " << duration.count() << " ms" << std::endl;

    QAM_demod qam_demod;
    auto demod_bits = qam_demod.demodulate(demod_signal);

    Segmenter segmenter;
    auto demod_bits_m = segmenter.reshape(demod_bits);
    demod_bits_m = segmenter.scramble(demod_bits_m);

    auto err_slots = segmenter.checkCRC(demod_bits_m);
    if (!err_slots.empty()) {
        std::cout << "Incorrect segments: ";
        for (auto err : err_slots) {
            std::cout << err << " ";
        }
        std::cout << std::endl;
    }

    // auto data = segmenter.extract_data(demod_bits_m);
    // auto flag = segmenter.extract_flag(demod_bits_m);

    // if      (flag == 1) bits2string(data);
    // else if (flag == 2) bits2file("test_file_out/", data);
    // //else if (flag == 3) sistem_inf_rx(data);

    return demod_bits_m;
}

void sistem_inf_rx(std::vector<uint8_t>& data){
    // Первыt 8 бит
    if (data.size() < 8) {
        std::cerr << "Недостаточно данных для извлечения числа." << std::endl;
        return;
    }
    uint8_t message = 0;
    for (int i = 0; i < 8; ++i) {
        message |= (data[i] << (7 - i));
    }

    std::cout << "Извлеченное число: 0x" << std::hex << static_cast<int>(message) << std::dec << std::endl;

    switch (message)
    {
    case 0:
        // 0 - Тут кто-нибудь есть?
        break;
    case 1:
        // 1 - Я здесь!
        break;
    case 2:
        // 2 Соединение установлено 
        break;
    case 3:
        // 3 - Конец связи
        break;
    
    case 10:
        // 10 - Принято без ошибок (ACK)
        break;
    case 15:
        // 15 - Приняты слоты с ошибкой (NACK)
        break;

    default:
        break;
    }
}

// Преобразование complex vector -> отдельные массивы real/imag/mag для ImPlot
void complexToArrays(const std::vector<cd>& signal, std::vector<float>& out_real, std::vector<float>& out_imag, std::vector<float>& out_mag) {
    out_real.resize(signal.size());
    out_imag.resize(signal.size());
    out_mag.resize(signal.size());
    for (size_t i = 0; i < signal.size(); ++i) {
        out_real[i] = static_cast<float>(signal[i].real());
        out_imag[i] = static_cast<float>(signal[i].imag());
        out_mag[i]  = static_cast<float>(std::abs(signal[i]));
    }
}

// Простое БПФ для спектрограммы (используем вашу реализацию)
std::vector<double> computeMagnitudeSpectrum(const std::vector<cd>& signal, int fft_size = 256) {
    std::vector<double> spectrum(fft_size / 2, 0.0);
    
    // Берём первые fft_size отсчётов (или меньше, если сигнал короче)
    int N = std::min(static_cast<int>(signal.size()), fft_size);
    std::vector<cd> windowed(N, 0.0);
    
    // Применяем окно Хэнна для уменьшения утечек
    for (int i = 0; i < N; ++i) {
        double hann = 0.5 * (1.0 - std::cos(2.0 * M_PI * i / (N - 1)));
        windowed[i] = signal[i] * hann;
    }
    
    // БПФ (используем вашу функцию)
    auto fft_result = fft(windowed); // false = прямой БПФ
    
    // Вычисляем магнитуду только для положительной частоты
    for (int i = 0; i < fft_size / 2; ++i) {
        spectrum[i] = std::abs(fft_result[i]);
    }
    
    return spectrum;
}

int main() {
    omp_set_num_threads(8);
    
    auto ofdm_data = transmission();
    auto noise_signal = add_ch(ofdm_data);
    noise_signal = std::vector(noise_signal.begin(), noise_signal.begin()+2600);
    auto demod_bits_m = receive(noise_signal);

    Segmenter segmenter;
    auto data = segmenter.extract_data(demod_bits_m);
    auto flag = segmenter.extract_flag(demod_bits_m);

    if      (flag == 1) bits2string(data);
    else if (flag == 2) bits2file("test_file_out/", data);


    // 1) Инициализация SDL
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
    SDL_Window* window = SDL_CreateWindow(
        "Backend start", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1024, 768, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);

    ImGui::CreateContext();
    ImPlot::CreateContext();

    // Ввод\вывод
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Включить Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Включить Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Включить Docking

    // 2.1) Привязка Imgui к SDL2 и OpenGl backend'ам
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init("#version 330");

    // 3) Игра началась
    bool running = true;


     while (running) {

        // 3.0) Обработка event'ов (inputs, window resize, mouse moving, etc.);
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            std::cout << "Processing some event: "<< event.type << std::endl;
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        // 3.1) Начинаем создавать новый фрейм;
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_None);


        // =============================================================================
// ГРАФИКИ (3 окна)
// =============================================================================

// --- Подготовка данных (делается один раз перед циклом, но для простоты здесь) ---
static std::vector<float> tx_real, tx_imag, tx_mag;
static std::vector<float> rx_real, rx_imag, rx_mag;
static std::vector<double> tx_spectrum, rx_spectrum;
static bool data_prepared = false;

if (!data_prepared) {
    complexToArrays(ofdm_data, tx_real, tx_imag, tx_mag);      // Переданный сигнал
    complexToArrays(noise_signal, rx_real, rx_imag, rx_mag);   // Зашумлённый сигнал
    tx_spectrum = computeMagnitudeSpectrum(ofdm_data);         // Спектр TX
    rx_spectrum = computeMagnitudeSpectrum(noise_signal);      // Спектр RX
    data_prepared = true;
}

// --- Окно 1: Переданный сигнал (TX) ---
ImGui::Begin("📡 Transmitted Signal (TX)");
if (ImPlot::BeginPlot("TX: Real & Imag", ImVec2(-1, 300))) {
    ImPlot::SetupAxis(ImAxis_X1, "Sample Index");
    ImPlot::SetupAxis(ImAxis_Y1, "Amplitude");
    ImPlot::SetupLegend(ImPlotLocation_NorthEast);
    
    // Отображаем только первые 500 отсчётов, чтобы не перегружать
    int plot_samples = std::min(500, static_cast<int>(tx_real.size()));
    ImPlot::PlotLine("Real", tx_real.data(), plot_samples);
    ImPlot::PlotLine("Imag", tx_imag.data(), plot_samples);
    ImPlot::EndPlot();
}
ImGui::End();

// --- Окно 2: Сигнал после канала (RX) ---
ImGui::Begin("📡 Received Signal (RX) with Noise");
if (ImPlot::BeginPlot("RX: Real & Imag", ImVec2(-1, 300))) {
    ImPlot::SetupAxis(ImAxis_X1, "Sample Index");
    ImPlot::SetupAxis(ImAxis_Y1, "Amplitude");
    ImPlot::SetupLegend(ImPlotLocation_NorthEast);
    
    int plot_samples = std::min(500, static_cast<int>(rx_real.size()));
    ImPlot::PlotLine("Real", rx_real.data(), plot_samples);
    ImPlot::PlotLine("Imag", rx_imag.data(), plot_samples);
    ImPlot::EndPlot();
}
ImGui::End();

// --- Окно 3: Спектрограмма (Amplitude Spectrum) ---
ImGui::Begin("📊 Amplitude Spectrum (FFT)");
if (ImPlot::BeginPlot("Spectrum: TX vs RX", ImVec2(-1, 300))) {
    ImPlot::SetupAxis(ImAxis_X1, "Frequency Bin");
    ImPlot::SetupAxis(ImAxis_Y1, "Magnitude");
    ImPlot::SetupLegend(ImPlotLocation_NorthEast);
    
    // Отображаем спектр (половина БПФ)
    ImPlot::PlotLine("TX Spectrum", tx_spectrum.data(), tx_spectrum.size());
    ImPlot::PlotLine("RX Spectrum", rx_spectrum.data(), rx_spectrum.size());
    ImPlot::EndPlot();
}
ImGui::End();

        // 3.3) Отправляем на рендер;
        ImGui::Render();
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(window);
    }

    // 4) Закрываем приложение безопасно.
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();






    // spectrogram_plot(noise_signal_cfo, "noise_signal_cfo");
    // cool_scatter(demod_signal, "demod_signal");
    // cool_plot(corr, "corr");
    // show_plot();

    return 0;
}
#include <fftw3.h>
#include <iostream>
#include <cmath>

int main() {
  // Инициализируйте массив входных данных
  double data[] = {1, 2, 3, 4, 5};
  const size_t N = sizeof(data) / sizeof(double);

  // Создайте массив для комплексных значений данных
  fftw_complex* fft_data = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);

  // Создайте объект плана преобразования Фурье
  fftw_plan plan = fftw_plan_dft_1d(N, fft_data, fft_data, FFTW_FORWARD, FFTW_ESTIMATE);

  // Скопируйте входные данные в массив комплексных значений
  for (size_t i = 0; i < N; ++i) {
    fft_data[i][0] = data[i];
    fft_data[i][1] = 0;
  }

  // Выполните преобразование Фурье
  fftw_execute(plan);

  // Освободите план
  fftw_destroy_plan(plan);

  // Получите результаты преобразования Фурье (комплексное значение)
  double fft_abs[N];
  double fft_phase[N];
  for (size_t i = 0; i < N; ++i) {
    fft_abs[i] = sqrt(fft_data[i][0] * fft_data[i][0] + fft_data[i][1] * fft_data[i][1]);
    fft_phase[i] = atan2(fft_data[i][1], fft_data[i][0]);
  }

  // Выведите амплитудный и фазовый спектры
  for (size_t i = 0; i < N; ++i) {
    std::cout << "Амплитуда частоты " << i << ": " << fft_abs[i] << std::endl;
    std::cout << "Фаза частоты " << i << ": " << fft_phase[i] << std::endl;
  }

  // Освободите массив комплексных значений
  fftw_free(fft_data);

  return 0;

}
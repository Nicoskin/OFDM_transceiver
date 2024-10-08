# Имя компилятора
CXX = g++

# Флаги компилятора
CXXFLAGS = -I"C:\mingw64\lib\fftw-3.3.5"

# Флаги линковщика
LDFLAGS = -L"C:\mingw64\lib" -lfftw3

# Исходные файлы
SRCS = ofdm_mod.cpp test.cpp

# Заголовочные файлы
HEADERS = ofdm_mod.h

# Имя выходного файла
TARGET = ofdm_program

# Правило для компиляции всех исходных файлов
$(TARGET): $(SRCS) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

# Правило для очистки скомпилированных файлов
clean:
	rm -f $(TARGET)

# Имя компилятора
CXX = g++

# Флаги компилятора
# CXXFLAGS = -I"C:\mingw64\lib\fftw-3.3.5"

# # Флаги линковщика
# LDFLAGS = -L"C:\mingw64\lib" -lfftw3

# Исходные файлы
SRCS =  test_seg_qam.cpp \
		../../Segmenter/segmenter.cpp \
		../../QAM_Modulation/modulation.cpp

# Заголовочные файлы
HEADERS = ../../Segmenter/segmenter.h \
		  ../../QAM_Modulation/modulation.h

# Имя выходного файла
TARGET = test

# Правило для компиляции всех исходных файлов
$(TARGET): $(SRCS) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

# Правило для очистки скомпилированных файлов
clean:
	rm -f $(TARGET)
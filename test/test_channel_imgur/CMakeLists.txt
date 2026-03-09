cmake_minimum_required(VERSION 3.10)

# Название проекта
project(test)

# Установить стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Флаги компиляции
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")

# Указать корневую директорию проекта (где лежит CMakeLists.txt)
set(PROJECT_ROOT ${CMAKE_SOURCE_DIR})

# =============================================================================
# ПУТИ К БИБЛИОТЕКАМ (ИСПРАВЛЕНО)
# =============================================================================
# Поднимаемся на 3 уровня вверх к корню OFDM_transceiver
set(THIRD_PARTY_DIR ${PROJECT_SOURCE_DIR}/../../third_party)

# Указать директории с заголовочными файлами
include_directories(
    ${PROJECT_ROOT}/../../QAM
    ${PROJECT_ROOT}/../../Segmenter
    ${PROJECT_ROOT}/../../OFDM
    ${PROJECT_ROOT}/../../OFDM/fft
    ${PROJECT_ROOT}/../../File_converter
    ${PROJECT_ROOT}/../../other
    ${THIRD_PARTY_DIR}/imgui
    ${THIRD_PARTY_DIR}/implot
)

# Указать исходные файлы
set(SOURCE_FILES
    test.cpp
    ../../QAM/qam_mod.cpp
    ../../QAM/qam_demod.cpp
    ../../Segmenter/segmenter.cpp
    ../../OFDM/ofdm_mod.cpp
    ../../OFDM/ofdm_demod.cpp
    ../../OFDM/fft/fft.cpp
    ../../OFDM/sequence.cpp
    ../../OFDM/freq_offset.cpp
    ../../File_converter/file_converter.cpp
    ../../other/model_channel.cpp
    ../../other/plots.cpp
)

# =============================================================================
# ПОИСК БИБЛИОТЕК (SDL2, OpenGL, GLEW, Python)
# =============================================================================

# SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# OpenGL/GLEW
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

# Python3 + NumPy
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
execute_process(
    COMMAND ${Python3_EXECUTABLE} -c "import numpy; print(numpy.get_include())"
    OUTPUT_VARIABLE NUMPY_INCLUDE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# =============================================================================
# СОЗДАНИЕ БИБЛИОТЕК IMGUI И IMPLOT
# =============================================================================

add_library(imgui STATIC
    ${THIRD_PARTY_DIR}/imgui/imgui.cpp
    ${THIRD_PARTY_DIR}/imgui/imgui_draw.cpp
    ${THIRD_PARTY_DIR}/imgui/imgui_demo.cpp
    ${THIRD_PARTY_DIR}/imgui/imgui_tables.cpp
    ${THIRD_PARTY_DIR}/imgui/imgui_widgets.cpp
    ${THIRD_PARTY_DIR}/imgui/backends/imgui_impl_sdl2.cpp
    ${THIRD_PARTY_DIR}/imgui/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC ${THIRD_PARTY_DIR}/imgui)
target_compile_options(imgui PRIVATE -fPIC)

add_library(implot STATIC
    ${THIRD_PARTY_DIR}/implot/implot.cpp
    ${THIRD_PARTY_DIR}/implot/implot_demo.cpp
    ${THIRD_PARTY_DIR}/implot/implot_items.cpp
)
target_include_directories(implot PUBLIC ${THIRD_PARTY_DIR}/implot)
target_link_libraries(implot PRIVATE imgui)
target_compile_options(implot PRIVATE -fPIC)

# =============================================================================
# СОЗДАНИЕ ИСПОЛНЯЕМОГО ФАЙЛА
# =============================================================================

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# Подключить заголовки Python И NumPy
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${Python3_INCLUDE_DIRS}
    ${NUMPY_INCLUDE_DIR}
)

# Линковка
target_link_libraries(${PROJECT_NAME} PRIVATE 
    ${Python3_LIBRARIES}
    imgui
    implot
    ${SDL2_LIBRARIES} 
    ${OPENGL_LIBRARIES} 
    ${GLEW_LIBRARIES}
)

# Копировать файлы (если папка существует)
if(EXISTS ${CMAKE_SOURCE_DIR}/test_file_in)
    file(COPY ${CMAKE_SOURCE_DIR}/test_file_in DESTINATION ${CMAKE_BINARY_DIR})
endif()

# Вывод для отладки
message(STATUS "Third party dir: ${THIRD_PARTY_DIR}")
message(STATUS "Python3 include: ${Python3_INCLUDE_DIRS}")
message(STATUS "NumPy include: ${NUMPY_INCLUDE_DIR}")
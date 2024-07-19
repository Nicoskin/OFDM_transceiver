import numpy as np
import matplotlib.pyplot as plt



def complex_cpp_file(file):

    with open(file, 'r') as file:
        data = file.read()
    # Убираем скобки и разделяем комплексные числа по запятым
    complex_numbers_str = data.replace('(', '').replace(')', '').split()

    complex_numbers = []
    for complex_number_str in complex_numbers_str:
        real_part, imag_part = map(float, complex_number_str.split(','))
        complex_number = complex(real_part, imag_part)
        complex_numbers.append(complex_number)

    rx = np.ravel(complex_numbers)
    return rx


rx = complex_cpp_file("/home/ivan/Desktop/Work_dir/1440/OFDM_transceiver/src/resurrs/rx_file_copy.txt")

plt.figure(1)
#plt.title("rx buffer 1 sec(2.5MS) | tx once (5760S)")
plt.title("rx buffer (10000S) | tx (10) (5760S) | ampl tx 2**14")

#plt.title("rx buffer 1 sec(2.5MS) | tx nothing")

plt.plot(abs(rx))
plt.show()
#print(fft[:10])

# if 1:
#     plt.figure()
#     plt.scatter(fft.real, fft.imag)
#     plt.show()


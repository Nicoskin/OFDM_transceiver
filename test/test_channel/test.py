import numpy as np
import matplotlib.pyplot as plt
import mylib as ml

def load_complex_numbers(filename):
    with open(filename, 'r') as file:
        # Parse each line as a pair of real and imaginary components
        lines = [line.strip().replace('(', '').replace(')', '').split(',') for line in file]
    
    # Convert lines to a numpy array of complex numbers
    complex_array = np.array([complex(float(real), float(imag)) for real, imag in lines])
    return complex_array

complex_array = load_complex_numbers("dem_sig.txt")
complex_array_qpsk = load_complex_numbers("qpsk.txt")
signal = load_complex_numbers("signal.txt")
#complex_array = complex_array[28:101]

# Plot the complex array
plt.figure(1, figsize=(10, 8))
plt.title("RX")
plt.plot(complex_array.real, '-', label="Real Part")
plt.plot(complex_array.imag, '-', label="Imaginary Part")
plt.legend()

ml.cool_plot(complex_array_qpsk.real, complex_array_qpsk.imag, title="TX QPSK")

ml.cool_plot(signal.real, signal.imag, title="signal")

ml.cool_scatter(complex_array.real, complex_array.imag, title="С интерполяцией по пилотам")

plt.show()
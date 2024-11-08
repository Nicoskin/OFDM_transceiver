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

dem_sig = load_complex_numbers("dem_sig.txt")
qpsk = load_complex_numbers("qpsk.txt")


ml.cool_plot(dem_sig.real, dem_sig.imag, title="dem_sig SNR=20dB CFO 1000",vid= '-o')
ml.cool_scatter(dem_sig.real, dem_sig.imag, title="dem_sig SNR=20dB CFO 1000")
ml.cool_plot(qpsk.real, qpsk.imag, title="qpsk", vid= '-o')

plt.show()
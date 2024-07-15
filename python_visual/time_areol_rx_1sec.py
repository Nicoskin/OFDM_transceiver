import numpy as np
import matplotlib.pyplot as plt
import time

real = np.loadtxt("/home/ivan/Desktop/Work_dir/1440/SDR_TX_RX/src/rx/real_rx.txt")


imag = np.loadtxt("/home/ivan/Desktop/Work_dir/1440/SDR_TX_RX/src/rx/imag_rx.txt")

rx = np.vectorize(complex)(real, imag)

print(len(rx))
print(rx[:10])

plt.figure(1)
#plt.title("rx buffer 1 sec(2.5MS) | tx once (5760S)")
#plt.title("rx buffer 1 sec(2.5MS) | tx cycle (5760S) | ampl tx 2**14")

plt.title("rx buffer 1 sec(2.5MS) | tx nothing")
plt.plot(abs(rx))
plt.show()
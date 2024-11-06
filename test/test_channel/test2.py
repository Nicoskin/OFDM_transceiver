import numpy as np
import matplotlib.pyplot as plt

arr = [
(-12685.9,1698.5),
(-165.25,-11436.1),
(-1201.13,-1623.2),
(-2535.21,9.16249),
(-2905.64,1697.62),
(-1369.77,959.969),
(-1902.43,-1126.81),
(1435.29,1623.12),
(1067.16,-1188.22),
(941.212,243.262),
(579.694,2406.72),
(10081.1,57.8379),
]

complex_data = np.array([complex(real, imag) for real, imag in arr])

plt.plot(complex_data, '-')
plt.plot(complex_data.imag, '-')
plt.show()
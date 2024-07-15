import numpy as np
import matplotlib.pyplot as plt
import time

conv = np.loadtxt("/home/ivan/Desktop/Work_dir/1440/SDR_TX_RX/src/rx/conv.txt")



print(conv[:10])


for r in range(1000):     #кол-во кадров с графиками
          # принятые данные помещаем в rx
    plt.clf()           # на каждой итерации очищение старого графика
    conv = np.loadtxt("/home/ivan/Desktop/Work_dir/1440/SDR_TX_RX/src/rx/conv.txt")
    plt.plot(conv)   
    plt.draw()          # пересовываем фигуру
    plt.xlabel('time')
    plt.ylabel('ampl')
    plt.pause(0.05)     # небольшая пауза перед отрисовкой, чтобы успеть обработать данные
    time.sleep(0.1) 
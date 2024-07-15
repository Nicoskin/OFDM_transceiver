#include <iostream>
#include <vector>
#include <complex> 
#include <iterator>
#include <fstream>
#include <cmath>
#include <unistd.h>
#include <chrono>

#include "../settings_sdr/settinds_sdr.h"
#include "../hdr/conv.h"

using namespace std;


int main (int argc, char **argv)
{
	struct iio_device *tx = context_tx(argc,argv);

    txbuf = iio_device_create_buffer(tx, 5760, false);

	if (!txbuf) {
		perror("Could not create TX buffer");
		shutdown();
	}
    char *p_dat, *p_end;
    ptrdiff_t p_inc;


	// файл с real частью
    FILE *real_file = fopen("/home/ivan/Desktop/Work_dir/Yadro/ofdm/real_part.txt", "r");
    if (real_file == NULL) {
        perror("Ошибка открытия файла с реальной частью");
        return EXIT_FAILURE;
    }

    // файл с мнимой частью
    FILE *imag_file = fopen("/home/ivan/Desktop/Work_dir/Yadro/ofdm/imag_part.txt", "r");
    if (imag_file == NULL) {
        perror("Ошибка открытия файла с мнимой частью");
        return EXIT_FAILURE;
    }


    // Прочитать данные из файлов
    double n;
	
    fscanf(real_file, "%lf\n", &n);
	int count = n;
	printf("n = %d\n", count);



    double *real_data = (double *)malloc(count* sizeof(double));
    double *imag_data = (double *)malloc(count * sizeof(double));
    for (int i = 0; i < n; i++) {
        fscanf(real_file, "%lf\n", &real_data[i]);
        fscanf(imag_file, "%lf\n", &imag_data[i]);
    }

    // Закрыть файлы
    fclose(real_file);
    fclose(imag_file);

    for (int i = 0; i < n; i++) {
      	//printf("%lf + %lfi\n", real_data[i], imag_data[i]);
   	}	
    int k  = 0;
    while(k < 100){
        k++;
        //stop = true;
        ssize_t nbytes_tx;
        nbytes_tx = iio_buffer_push(txbuf);
		if (nbytes_tx < 0) { printf("Error pushing buf %d\n", (int) nbytes_tx); shutdown(); }

		//printf("nbit_tx = %ld\n", nbytes_tx);



    	p_inc = iio_buffer_step(txbuf);
		p_end = (char*)iio_buffer_end(txbuf);

        auto start = std::chrono::high_resolution_clock::now();
    
		int i = 0;
		for (p_dat = (char *)iio_buffer_first(txbuf, tx0_i); p_dat < p_end; p_dat += p_inc) {
			
			// Example: fill with zeros
			// 12-bit sample needs to be MSB aligned so shift by 4
			// https://wiki.analog.com/resources/eval/user-guides/ad-fmcomms2-ebz/software/basic_iq_datafiles#binary_format
			//((int16_t*)p_dat)[0] = 1; // Real (I)
			//((int16_t*)p_dat)[1] = 1; // Imag (Q)
			((int16_t*)p_dat)[0] = real_data[i]; // Real (I)
			((int16_t*)p_dat)[1] = imag_data[i]; // Imag (Q)
			//printf("i_tx = %d\n", ((int16_t*)p_dat)[0]);
			//printf("q_tx = %d\n", ((int16_t*)p_dat)[1]);
			i++;
            
	
		}
        //usleep(1000);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        cout << "Time tx: " << duration.count() << endl;
		//printf("Time: %d\n\n", duration.count());

       //sleep(0.5);
    }
    shutdown();

}


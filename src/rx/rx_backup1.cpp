#include <iostream>
#include <vector>
#include <complex> 
#include <iterator>
#include <fstream>
#include <cmath>

#include "../settings_sdr/settinds_sdr.h"
#include "../hdr/conv.h"

using namespace std;


bool check_pss(vector <double> conv)
{
	int flag = 0;
	for(int i = 0; i < conv.size(); i++){

		if(conv[i] > 0.8){
			flag++;
			//cout << "true" << endl;

			//return true;
		}

	}
	//cout << flag << endl;
	if(flag == 12){
		return true;
	}
}


int main (int argc, char **argv)
{
	// struct iio_device *tx = context_tx(argc,argv);
	struct iio_device *rx = context_rx(argc,argv);


	rxbuf = iio_device_create_buffer(rx, 5670, false);
	//cout << "rxbuf " << rxbuf << endl;
	if (!rxbuf) {
		perror("Could not create RX buffer");
		shutdown();
	}
	// txbuf = iio_device_create_buffer(tx, 5760, false);
	// if (!txbuf) {
	// 	perror("Could not create TX buffer");
	// 	shutdown();
	// }

	//cout << " tx : " << (int*)txbuf->buffer << endl;
	//cout << " tx : " *((int*)txbuf->buffer) << endl;
	//for (int i = 0; i < 10; i ++ ){
	//	int *ptr = (int*)txbuf;
	//	printf("tx - buf : %d \n", *ptr);
	//	ptr++;
	//}




	printf("* Starting IO streaming (press CTRL+C to cancel)\n");


	if (1){
		int k = 0;
	while (!stop)
	{	

		ssize_t nbytes_rx, nbytes_tx;
		char *p_dat, *p_end;
		ptrdiff_t p_inc;
		stop = true;
 
		// Schedule TX buffer
		// nbytes_tx = iio_buffer_push(txbuf);
		// if (nbytes_tx < 0) { printf("Error pushing buf %d\n", (int) nbytes_tx); shutdown(); }

		// printf("nbit_tx = %ld\n", nbytes_tx);

		// Refill RX buffer//Пополнить буфер приема
		nbytes_rx = iio_buffer_refill(rxbuf);
		if (nbytes_rx < 0) { printf("Error refilling buf %d\n",(int) nbytes_rx); shutdown(); }

		//printf("nbit_rx = %ld\n", nbytes_rx);

		// READ: Get pointers to RX buf and read IQ from RX buf port 0//Получите указатели на RX buf и прочитайте IQ из порта RX buf 0
		p_inc = iio_buffer_step(rxbuf);
		p_end = (char*)iio_buffer_end(rxbuf);
		int count = 0;

		
		FILE *real_file_rx = fopen("real_rx.txt", "w");
		if (real_file_rx == NULL) {
			perror("Ошибка открытия файла с реальной частью");
			return EXIT_FAILURE;
		}

		FILE *imag_file_rx = fopen("imag_rx.txt", "w");
		if (imag_file_rx == NULL) {
			perror("Ошибка открытия файла с мнимой частью");
			return EXIT_FAILURE;
		}

		vector <complex<double>> rx_data;


	    int c_nt = 5760 * 1;
		int n = 0;
		for (p_dat = (char *)iio_buffer_first(rxbuf, rx0_i); p_dat < p_end; p_dat += p_inc) {
			//cout << "p dat " << ((int16_t*)p_dat)[0]<< endl;
			//cout << "p inc " <<  p_inc << endl;
			// Example: swap I and Q
			const int16_t i = ((int16_t*)p_dat)[0]; // Real (I)
			const int16_t q = ((int16_t*)p_dat)[1]; // Imag (Q)
			((int16_t*)p_dat)[0] = q;
			((int16_t*)p_dat)[1] = i;
			rx_data.push_back(complex<double>(i, q));


			fprintf(real_file_rx, "%d\n", i);
			fprintf(imag_file_rx, "%d\n", q);
			n++;


			//printf("i = %d\n", i);
			//printf("q = %d\n", q);
			//count++;
		}
			
		ifstream real_file1("/home/ivan/Desktop/Work_dir/Yadro/ofdm/pss_real.txt");
		if (!real_file1.is_open()) {
			std::cerr << "Не удалось открыть файл1" << std::endl;
			return 1;
		}
		// файл с мнимой частью
		ifstream imag_file1("/home/ivan/Desktop/Work_dir/Yadro/ofdm/pss_imag.txt");
		if (!imag_file1.is_open()) {
			std::cerr << "Не удалось открыть файл1" << std::endl;
			return 1;
		}


		vector<complex<double>> pss_ifft;

		double real_num, imag_num;
		while (real_file1 >> real_num && imag_file1 >> imag_num) {
			pss_ifft.push_back(complex<double>(real_num, imag_num));
		}
		
		for(complex<double> num:rx_data){
			//cout << num <<  ' ';
		}

		real_file1.close();
		imag_file1.close();

		vector<complex<double>> conjugate_pss;
		for (complex<double> number : pss_ifft) {
			conjugate_pss.push_back(conj(number));
		}
		//cout << "\n\n\n"<<endl;


		
		vector<double> conv = convolve(rx_data, conjugate_pss);

		

		FILE *conv_file = fopen("conv.txt", "w");
		if (conv_file == NULL) {
			perror("conv");
			return EXIT_FAILURE;
		}
		for (double number : conv) {
			fprintf(conv_file, "%f\n", number);
		}
		stop = check_pss(conv);
		//printf("countttt --- %d\n\n", count);
		fclose(conv_file);
		fclose(real_file_rx);
		fclose(imag_file_rx);


		// WRITE:	 Get pointers to TX buf and write IQ to TX buf port 0
		/*if(0){
		p_inc = iio_buffer_step(txbuf);
		p_end = (char*)iio_buffer_end(txbuf);
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
			count++;
		}
		printf("countttt --- %d\n\n", count);
		}

		*/
		// Sample counter increment and status output
		//nrx += nbytes_rx / iio_device_get_sample_size(rx);
		//ntx += nbytes_tx / iio_device_get_sample_size(tx);
		//printf("\tRX %8.2f MSmp, TX %8.2f MSmp\n", nrx/1e6, ntx/1e6);
	}

	shutdown();

	};

	return 0;
}
#include <iostream>
#include <vector>
#include <complex> 
#include <iterator>
#include <fstream>
#include <cmath>

#include "../settings_sdr/settinds_sdr.h"
#include "../hdr/conv.h"

using namespace std;

int main (int argc, char **argv)
{
	// Streaming devices
	struct iio_device *tx;
	struct iio_device *rx;

	// RX and TX sample counters
	size_t nrx = 0;
	size_t ntx = 0;

	// Stream configurations
	struct stream_cfg rxcfg;
	struct stream_cfg txcfg;

	// Listen to ctrl+c and IIO_ENSURE
	signal(SIGINT, handle_sig);

	// RX stream config
	rxcfg.bw_hz = MHZ(2);   // 2 MHz rf bandwidth
	rxcfg.fs_hz = MHZ(2.5);   // 2.5 MS/s rx sample rate
	rxcfg.lo_hz = GHZ(2.3); // 2.5 GHz rf frequency
	rxcfg.rfport = "A_BALANCED"; // port A (select for rf freq.)

	// TX stream config
	txcfg.bw_hz = MHZ(1.5); // 1.5 MHz rf bandwidth
	txcfg.fs_hz = MHZ(2.5);   // 2.5 MS/s tx sample rate
	txcfg.lo_hz = GHZ(2.3); // 2.5 GHz rf frequency
	txcfg.rfport = "A"; // port A (select for rf freq.)

	printf("* Acquiring IIO context\n");
	if (argc == 1) {
		IIO_ENSURE((ctx = iio_create_default_context()) && "No context");
	}
	else if (argc == 2) {
		IIO_ENSURE((ctx = iio_create_context_from_uri(argv[1])) && "No context");
	}
	IIO_ENSURE(iio_context_get_devices_count(ctx) > 0 && "No devices");

	printf("* Acquiring AD9361 streaming devices\n");
	IIO_ENSURE(get_ad9361_stream_dev(TX, &tx) && "No tx dev found");
	IIO_ENSURE(get_ad9361_stream_dev(RX, &rx) && "No rx dev found");

	printf("* Configuring AD9361 for streaming\n");
	IIO_ENSURE(cfg_ad9361_streaming_ch(&rxcfg, RX, 0) && "RX port 0 not found");
	IIO_ENSURE(cfg_ad9361_streaming_ch(&txcfg, TX, 0) && "TX port 0 not found");

	printf("* Initializing AD9361 IIO streaming channels\n");
	IIO_ENSURE(get_ad9361_stream_ch(RX, rx, 0, &rx0_i) && "RX chan i not found");
	IIO_ENSURE(get_ad9361_stream_ch(RX, rx, 1, &rx0_q) && "RX chan q not found");
	IIO_ENSURE(get_ad9361_stream_ch(TX, tx, 0, &tx0_i) && "TX chan i not found");
	IIO_ENSURE(get_ad9361_stream_ch(TX, tx, 1, &tx0_q) && "TX chan q not found");

	printf("* Enabling IIO streaming channels\n");
	iio_channel_enable(rx0_i);
	iio_channel_enable(rx0_q);
	iio_channel_enable(tx0_i);
	iio_channel_enable(tx0_q);

	printf("* Creating non-cyclic IIO buffers with 1 MiS\n");

	//int size_buffer = 

	rxbuf = iio_device_create_buffer(rx, 5760*2, false);
	//cout << "rxbuf " << rxbuf << endl;
	if (!rxbuf) {
		perror("Could not create RX buffer");
		shutdown();
	}
	txbuf = iio_device_create_buffer(tx, int(5760), false);
	if (!txbuf) {
		perror("Could not create TX buffer");
		shutdown();
	}

	//cout << " tx : " << (int*)txbuf->buffer << endl;
	//cout << " tx : " *((int*)txbuf->buffer) << endl;
	//for (int i = 0; i < 10; i ++ ){
	//	int *ptr = (int*)txbuf;
	//	printf("tx - buf : %d \n", *ptr);
	//	ptr++;
	//}


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

	ofstream file("/home/ivan/Desktop/Work_dir/1440/OFDM_transceiver/src/resurrs/rx_file_copy.txt", ios::binary); 



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

	printf("* Starting IO streaming (press CTRL+C to cancel)\n");

	int k = 0;
	vector <complex<double>> rx_data;
	if (1){
	while (k<5)
	{
		k++;
		ssize_t nbytes_rx, nbytes_tx;
		char *p_dat, *p_end;
		ptrdiff_t p_inc;
		
 
		// Schedule TX buffer
		


		//printf("nbit_tx = %ld\n", nbytes_tx);

		// Refill RX buffer//Пополнить буфер приема


		// READ: Get pointers to RX buf and read IQ from RX buf port 0//Получите указатели на RX buf и прочитайте IQ из порта RX buf 0
		
		int count = 0;
	

		
		if(k >= 3){
			nbytes_tx = iio_buffer_push(txbuf);
			if (nbytes_tx < 0) { printf("Error pushing buf %d\n", (int) nbytes_tx); shutdown(); }
			
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
	
		
		// FILE *real_file_rx = fopen("real_rx.txt", "w");
		// if (real_file_rx == NULL) {
		// 	perror("Ошибка открытия файла с реальной частью");
		// 	return EXIT_FAILURE;
		// }

		// FILE *imag_file_rx = fopen("imag_rx.txt", "w");
		// if (imag_file_rx == NULL) {
		// 	perror("Ошибка открытия файла с мнимой частью");
		// 	return EXIT_FAILURE;
		// }
		if (1){
			nbytes_rx = iio_buffer_refill(rxbuf);
			if (nbytes_rx < 0) { printf("Error refilling buf %d\n",(int) nbytes_rx); shutdown(); }

			printf("nbit_rx = %ld\n", nbytes_rx);

			p_inc = iio_buffer_step(rxbuf);
			p_end = (char*)iio_buffer_end(rxbuf);
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


				//fprintf(real_file_rx, "%d\n", i);
				//fprintf(imag_file_rx, "%d\n", q);
				n++;


				//printf("i = %d\n", i);
				//printf("q = %d\n", q);
				//count++;
			}
		}
		// ifstream real_file1("/home/ivan/Desktop/Work_dir/Yadro/ofdm/pss_real.txt");
		// if (!real_file1.is_open()) {
		// 	std::cerr << "Не удалось открыть файл1" << std::endl;
		// 	return 1;
		// }
		// // файл с мнимой частью
		// ifstream imag_file1("/home/ivan/Desktop/Work_dir/Yadro/ofdm/pss_imag.txt");
		// if (!imag_file1.is_open()) {
		// 	std::cerr << "Не удалось открыть файл1" << std::endl;
		// 	return 1;
		// }


		// vector<complex<double>> pss_ifft;

		// double real_num, imag_num;
		// while (real_file1 >> real_num && imag_file1 >> imag_num) {
		// 	pss_ifft.push_back(complex<double>(real_num, imag_num));
		// }
		
		// for(complex<double> num:rx_data){
		// 	//cout << num <<  ' ';
		// }

		// real_file1.close();
		// imag_file1.close();

		// vector<complex<double>> conjugate_pss;
		// for (complex<double> number : pss_ifft) {
		// 	conjugate_pss.push_back(conj(number));
		// }
		// cout << "\n\n\n"<<endl;


		
		// vector<double> conv = convolve(rx_data, conjugate_pss);

		// FILE *conv_file = fopen("conv.txt", "w");
		// if (conv_file == NULL) {
		// 	perror("conv");
		// 	return EXIT_FAILURE;
		// }
		// for (double number : conv) {
		// 	fprintf(conv_file, "%f\n", number);
		// }

		// //printf("countttt --- %d\n\n", count);
		// fclose(conv_file);
		// fclose(real_file_rx);
		// fclose(imag_file_rx);


		// WRITE:	 Get pointers to TX buf and write IQ to TX buf port 0

		// Sample counter increment and status output
		nrx += nbytes_rx / iio_device_get_sample_size(rx);
		ntx += nbytes_tx / iio_device_get_sample_size(tx);
		//printf("\tRX %8.2f MSmp, TX %8.2f MSmp\n", nrx/1e6, ntx/1e6);
	}
	if (file.is_open()){
        for( int i = 0; i < rx_data.size(); i++){
            file << rx_data[i] << " ";
        }
    }
	file.close();
	shutdown();
	free(real_data);
    free(imag_data);
	};
	return 0;
}
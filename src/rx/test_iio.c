/*
 * libiio - AD9361 IIO streaming example
 *
 * Copyright (C) 2014 IABG mbH
 * Author: Michael Feilen <feilen_at_iabg.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **/

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <iio.h>
//#include <iostream>


//using namespace std;
/* helper macros */
#define MHZ(x) ((long long)(x*1000000.0 + .5))
#define GHZ(x) ((long long)(x*1000000000.0 + .5))

#define IIO_ENSURE(expr) { \
	if (!(expr)) { \
		(void) fprintf(stderr, "assertion failed (%s:%d)\n", __FILE__, __LINE__); \
		(void) abort(); \
	} \
}

/* RX is input, TX is output */
enum iodev { RX, TX };

/* common RX and TX streaming params */
struct stream_cfg {
	long long bw_hz; // Analog banwidth in Hz
	long long fs_hz; // Baseband sample rate in Hz
	long long lo_hz; // Local oscillator frequency in Hz
	const char* rfport; // Port name
};

/* static scratch mem for strings */
static char tmpstr[64];

/* IIO structs required for streaming */
static struct iio_context *ctx   = NULL;
static struct iio_channel *rx0_i = NULL;
static struct iio_channel *rx0_q = NULL;
static struct iio_channel *tx0_i = NULL;
static struct iio_channel *tx0_q = NULL;
static struct iio_buffer  *rxbuf = NULL;
struct iio_buffer  *txbuf = NULL;

static bool stop;

/* cleanup and exit */
static void shutdown()
{
	printf("* Destroying buffers\n");
	if (rxbuf) { iio_buffer_destroy(rxbuf); }
	if (txbuf) { iio_buffer_destroy(txbuf); }

	printf("* Disabling streaming channels\n");
	if (rx0_i) { iio_channel_disable(rx0_i); }
	if (rx0_q) { iio_channel_disable(rx0_q); }
	if (tx0_i) { iio_channel_disable(tx0_i); }
	if (tx0_q) { iio_channel_disable(tx0_q); }

	printf("* Destroying context\n");
	if (ctx) { iio_context_destroy(ctx); }
	exit(0);
}

static void handle_sig(int sig)
{
	printf("Waiting for process to finish... Got signal %d\n", sig);
	stop = true;
}

/* check return value of attr_write function */
static void errchk(int v, const char* what) {
	 if (v < 0) { fprintf(stderr, "Error %d writing to channel \"%s\"\nvalue may not be supported.\n", v, what); shutdown(); }
}

/* write attribute: long long int */
static void wr_ch_lli(struct iio_channel *chn, const char* what, long long val)
{
	errchk(iio_channel_attr_write_longlong(chn, what, val), what);
}

/* write attribute: string */
static void wr_ch_str(struct iio_channel *chn, const char* what, const char* str)
{
	errchk(iio_channel_attr_write(chn, what, str), what);
}

/* helper function generating channel names */
static char* get_ch_name(const char* type, int id)
{
	snprintf(tmpstr, sizeof(tmpstr), "%s%d", type, id);
	return tmpstr;
}

/* returns ad9361 phy device */
static struct iio_device* get_ad9361_phy(void)
{
	struct iio_device *dev =  iio_context_find_device(ctx, "ad9361-phy");
	IIO_ENSURE(dev && "No ad9361-phy found");
	return dev;
}

/* finds AD9361 streaming IIO devices */
static bool get_ad9361_stream_dev(enum iodev d, struct iio_device **dev)
{
	switch (d) {
	case TX: *dev = iio_context_find_device(ctx, "cf-ad9361-dds-core-lpc"); return *dev != NULL;
	case RX: *dev = iio_context_find_device(ctx, "cf-ad9361-lpc");  return *dev != NULL;
	default: IIO_ENSURE(0); return false;
	}
}

/* finds AD9361 streaming IIO channels */
static bool get_ad9361_stream_ch(enum iodev d, struct iio_device *dev, int chid, struct iio_channel **chn)
{
	*chn = iio_device_find_channel(dev, get_ch_name("voltage", chid), d == TX);
	if (!*chn)
		*chn = iio_device_find_channel(dev, get_ch_name("altvoltage", chid), d == TX);
	return *chn != NULL;
}

/* finds AD9361 phy IIO configuration channel with id chid */
static bool get_phy_chan(enum iodev d, int chid, struct iio_channel **chn)
{
	switch (d) {
	case RX: *chn = iio_device_find_channel(get_ad9361_phy(), get_ch_name("voltage", chid), false); return *chn != NULL;
	case TX: *chn = iio_device_find_channel(get_ad9361_phy(), get_ch_name("voltage", chid), true);  return *chn != NULL;
	default: IIO_ENSURE(0); return false;
	}
}

/* finds AD9361 local oscillator IIO configuration channels */
static bool get_lo_chan(enum iodev d, struct iio_channel **chn)
{
	switch (d) {
	 // LO chan is always output, i.e. true
	case RX: *chn = iio_device_find_channel(get_ad9361_phy(), get_ch_name("altvoltage", 0), true); return *chn != NULL;
	case TX: *chn = iio_device_find_channel(get_ad9361_phy(), get_ch_name("altvoltage", 1), true); return *chn != NULL;
	default: IIO_ENSURE(0); return false;
	}
}

/* applies streaming configuration through IIO */
bool cfg_ad9361_streaming_ch(struct stream_cfg *cfg, enum iodev type, int chid)
{
	struct iio_channel *chn = NULL;

	// Configure phy and lo channels
	printf("* Acquiring AD9361 phy channel %d\n", chid);
	if (!get_phy_chan(type, chid, &chn)) {	return false; }
	wr_ch_str(chn, "rf_port_select",     cfg->rfport);
	wr_ch_lli(chn, "rf_bandwidth",       cfg->bw_hz);
	wr_ch_lli(chn, "sampling_frequency", cfg->fs_hz);

	// Configure LO channel
	printf("* Acquiring AD9361 %s lo channel\n", type == TX ? "TX" : "RX");
	if (!get_lo_chan(type, &chn)) { return false; }
	wr_ch_lli(chn, "frequency", cfg->lo_hz);
	return true;
}

/* simple configuration and streaming */
/* usage:
 * Default context, assuming local IIO devices, i.e., this script is run on ADALM-Pluto for example
 $./a.out
 * URI context, find out the uri by typing `iio_info -s` at the command line of the host PC
 $./a.out usb:x.x.x
 */
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
	rxcfg.lo_hz = GHZ(2.5); // 2.5 GHz rf frequency
	rxcfg.rfport = "A_BALANCED"; // port A (select for rf freq.)

	// TX stream config
	txcfg.bw_hz = MHZ(1.5); // 1.5 MHz rf bandwidth
	txcfg.fs_hz = MHZ(2.5);   // 2.5 MS/s tx sample rate
	txcfg.lo_hz = GHZ(2.5); // 2.5 GHz rf frequency
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

	rxbuf = iio_device_create_buffer(rx, 5760*3, false);
	//cout << "rxbuf " << rxbuf << endl;
	if (!rxbuf) {
		perror("Could not create RX buffer");
		shutdown();
	}
	txbuf = iio_device_create_buffer(tx, 5760, false);
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
    FILE *real_file = fopen("/home/ivan/Desktop/Work_dir/Yadro/ofdm/OFDM_TX_RX/real_part.txt", "r");
    if (real_file == NULL) {
        perror("Ошибка открытия файла с реальной частью");
        return EXIT_FAILURE;
    }

    // файл с мнимой частью
    FILE *imag_file = fopen("/home/ivan/Desktop/Work_dir/Yadro/ofdm/OFDM_TX_RX/imag_part.txt", "r");
    if (imag_file == NULL) {
        perror("Ошибка открытия файла с мнимой частью");
        return EXIT_FAILURE;
    }


 // Прочитать данные из файлов
    double n;
	
    fscanf(real_file, "%lf\n", &n);
	int count = n;
	printf("n = %d\n", count);





    double *real_data = malloc(count* sizeof(double));
    double *imag_data = malloc(count * sizeof(double));
    for (int i = 0; i < n; i++) {
        fscanf(real_file, "%lf\n", &real_data[i]);
        fscanf(imag_file, "%lf\n", &imag_data[i]);
    }

    // Закрыть файлы
    fclose(real_file);
    fclose(imag_file);

    for (int i = 0; i < n; i++) {
      	printf("%lf + %lfi\n", real_data[i], imag_data[i]);
   	}	

	printf("* Starting IO streaming (press CTRL+C to cancel)\n");
	if (1){
	while (!stop)
	{
		ssize_t nbytes_rx, nbytes_tx;
		char *p_dat, *p_end;
		ptrdiff_t p_inc;
		
 
		// Schedule TX buffer
		nbytes_tx = iio_buffer_push(txbuf);
		if (nbytes_tx < 0) { printf("Error pushing buf %d\n", (int) nbytes_tx); shutdown(); }

		printf("nbit_tx = %ld\n", nbytes_tx);

		// Refill RX buffer//Пополнить буфер приема
		nbytes_rx = iio_buffer_refill(rxbuf);
		if (nbytes_rx < 0) { printf("Error refilling buf %d\n",(int) nbytes_rx); shutdown(); }

		printf("nbit_rx = %ld\n", nbytes_rx);

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

	    int c_nt = 5760 * 3;
		int n = 0;
		for (p_dat = (char *)iio_buffer_first(rxbuf, rx0_i); p_dat < p_end; p_dat += p_inc) {
			//cout << "p dat " << ((int16_t*)p_dat)[0]<< endl;
			//cout << "p inc " <<  p_inc << endl;
			// Example: swap I and Q
			const int16_t i = ((int16_t*)p_dat)[0]; // Real (I)
			const int16_t q = ((int16_t*)p_dat)[1]; // Imag (Q)
			((int16_t*)p_dat)[0] = q;
			((int16_t*)p_dat)[1] = i;
			
			fprintf(real_file_rx, "%d\n", i);
			fprintf(imag_file_rx, "%d\n", q);
			n++;


			//printf("i = %d\n", i);
			//printf("q = %d\n", q);
			//count++;
		}
		printf("countttt --- %d\n\n", count);
		
		fclose(real_file_rx);
		fclose(imag_file_rx);


		// WRITE:	 Get pointers to TX buf and write IQ to TX buf port 0
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
		// Sample counter increment and status output
		nrx += nbytes_rx / iio_device_get_sample_size(rx);
		ntx += nbytes_tx / iio_device_get_sample_size(tx);
		//printf("\tRX %8.2f MSmp, TX %8.2f MSmp\n", nrx/1e6, ntx/1e6);
	}

	shutdown();
	free(real_data);
    free(imag_data);
	};
	return 0;
}

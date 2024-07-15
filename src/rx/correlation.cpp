#include <iostream>
#include <vector>
#include <complex> 
#include <iterator>
#include <fstream>
#include <cmath>

#include <SFML/Graphics.hpp>

// g++ plot.cpp -o l -lsfml-graphics -lsfml-window -lsfml-syste

using namespace std;



vector<complex<double>> zadoff_chu(int N = 1, int u = 25, bool PSS = false) {
    if (PSS) {

        N = 63;
        vector<complex<double>> ex1(31);

        for (int n = 0; n < 31; ++n) {
            ex1[n] = exp(-1i * M_PI * double(u) * double(n) * double(n + 1) / double(N));
        }

        vector<complex<double>> ex2(31);

        for (int n = 31; n < 62; ++n) {
            ex2[n - 31] = exp(-1i * M_PI * double(u) * double((n + 1)) * double(n + 2) / double(N));
        }

        vector<complex<double>> result(62);

        copy(ex1.begin(), ex1.end(), result.begin());
        copy(ex2.begin(), ex2.end(), result.begin() + 31);

        return result;
    }    
    else {
        vector<complex<double>> result(N);

        for (int n = 0; n < N; ++n) {
            result[n] = exp(-1i * M_PI * double(u) * double(n) * double((n + 1)) / double(N));
        }

        return result;
    }
}


vector<complex<double>> pss_on_carrier(int Nfft){

    

    ifstream real_file("/home/ivan/Desktop/Work_dir/Yadro/ofdm/OFDM_TX_RX/pss_real.txt");
    if (!real_file.is_open()) {
        std::cerr << "Не удалось открыть файл2" << std::endl;
        
    }
    // файл с мнимой частью
    ifstream imag_file("/home/ivan/Desktop/Work_dir/Yadro/ofdm/OFDM_TX_RX/pss_imag.txt");
    if (!imag_file.is_open()) {
        std::cerr << "Не удалось открыть файл2" << std::endl;
        
    }

    vector<double> real_data;
    vector<double> imag_data;
    vector<complex<double>> pss_ifft;

    double real_num, imag_num;
    while (real_file >> real_num && imag_file >> imag_num) {
        pss_ifft.push_back(complex<double>(real_num, imag_num));
    }
    
    for(complex<double> num:pss_ifft){
        cout << num <<  ' ';
    }

    real_file.close();
    imag_file.close();

    return pss_ifft;
}


vector<double> convolve(vector<complex<double>>x, vector<complex<double>>h) {
    
    int n = x.size() + h.size() - 1;
    cout << h.size() << " " << x.size() << endl;
    vector<complex<double>> y(n);

    x.insert(x.begin(), h.size()-1, complex<double>(0, 0));
        
    // cout << y.size() << endl;
    // cout << x.size() << endl;
    complex <double> norm_x;
    complex <double> norm_h;

    for(int i = 0; i < x.size(); i ++){
        norm_x = x[i] * x[i];
    }
    for(int i = 0; i < h.size(); i ++){
        norm_h = h[i] * h[i];
    }
    

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < h.size(); j++) {       
            y[i] += x[i+j] * h[j];
        }
        y[i] = (y[i] / (sqrt(norm_h) * sqrt(norm_x)));
    }
    vector <double> abs_y(n);
    int count = 0;
    for (int i = 0; i < n; i++){
        abs_y[i] = abs(y[i]);
        if (abs_y[i]> 7000){
            count++;
        }
    }
    //cout << "conv1 = " << count << endl;


    return abs_y;
}

vector<complex<double>> convolve2(vector<complex<double>>x, vector<complex<double>>h) {
    
    int n = x.size() + h.size() - 1;
    cout << h.size() << " " << x.size() << endl;
    vector<complex<double>> y(n);

    x.insert(x.begin(), h.size()-1, complex<double>(0, 0));
        
    // cout << y.size() << endl;
    // cout << x.size() << endl;
    int count = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < h.size(); j++){       
            y[i] += x[i+j] * h[j];

            
        }
    }

    vector <double> abs_y(n);

    for (int i = 0; i < n; i++){
        //abs_y[i] = sqrt(y[i].real()*y[i].real() + y[i].imag()*y[i].imag());
    }
    cout << "count > 9k " << count<< endl;

    return y;
}





int main(){


    ifstream real_file("/home/ivan/Desktop/Work_dir/Yadro/ofdm/real_part.txt");
    if (!real_file.is_open()) {
        std::cerr << "Не удалось открыть файл3" << std::endl;
        return 1;
    }
    // файл с мнимой частью
    ifstream imag_file("/home/ivan/Desktop/Work_dir/Yadro/ofdm/imag_part.txt");
    if (!imag_file.is_open()) {
        std::cerr << "Не удалось открыть файл3" << std::endl;
        return 1;
    }



    vector <double> real_data;
    vector <double> imag_data;

    // istream_iterator<double> it_real(real_file);
    // istream_iterator<double> it_imag(imag_file);
    // copy(it_real, istream_iterator<double>(), back_inserter(real_data));
    // copy(it_imag, istream_iterator<double>(), back_inserter(imag_data));


    vector<complex<double>> rx_data;

    double real, imag;
    while (real_file >> real && imag_file >> imag) {
        rx_data.push_back(complex<double>(real, imag));
    }

    // Закрыть файлы
    real_file.close();
    imag_file.close();

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
    cout << "\n\n\n"<<endl;


    
    vector<double> conv = convolve(rx_data, conjugate_pss);
    for (complex<double> number : conv) {
        //cout << number << " ";
    }

    vector<complex<double>> conv2 = convolve2(rx_data, conjugate_pss);


    
}

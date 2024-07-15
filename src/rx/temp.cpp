#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <vector>
#include <iostream>
#include <QVector>
#include <complex>
#include <iterator>
#include <fstream>

using namespace std;

QVector <double> conv(){
    ifstream real_file("/home/ivan/Desktop/Work_dir/Yadro/ofdm/test_plot_qt.txt");
    if (!real_file.is_open()) {
        std::cerr << "Не удалось открыть файл1" << std::endl;
    }
    double num;
    QVector <double> convol;
    while (real_file >> num ) {
        convol.push_back(num);
    }
    for(double num1:convol){
        //cout << num1 <<  ' ';
    }
    return convol;
}


vector <complex<double>> Correlations(vector <complex<double>> a, vector <complex<double>> b){

    int n = a.size() + b.size() - 1;
    vector <complex<double>> y(n);
    int size_b = b.size();

    for (int i = 0; i < (size_b - 1); i++) {
        a.insert(a.begin(), complex<double>(0, 0));
    }


    for (int i = 0; i < n; i++){
        for(int j = 0; j < size_b; j++ ){

            y[i]+= a[i+j]*b[j] ;

        }
    }


    return y;
}






QVector<complex<double>> pss_on_carrier(int Nfft){



    ifstream real_file("/home/ivan/Desktop/Work_dir/Yadro/ofdm/pss_real.txt");
    if (!real_file.is_open()) {
        std::cerr << "Не удалось открыть файл" << std::endl;

    }
    // файл с мнимой частью
    ifstream imag_file("/home/ivan/Desktop/Work_dir/Yadro/ofdm/pss_imag.txt");
    if (!imag_file.is_open()) {
        std::cerr << "Не удалось открыть файл" << std::endl;

    }

    vector<double> real_data;
    vector<double> imag_data;
    QVector<complex<double>> pss_ifft;

    double real_num, imag_num;
    while (real_file >> real_num && imag_file >> imag_num) {
        pss_ifft.push_back(complex<double>(real_num, imag_num));
    }

    for(complex<double> num:pss_ifft){
       // cout << num <<  ' ';
    }

    real_file.close();
    imag_file.close();

    return pss_ifft;
}


QVector <complex<double>> rx_data(){
    ifstream real_file("/home/ivan/Desktop/Work_dir/Yadro/ofdm/real_part.txt");
    if (!real_file.is_open()) {
        std::cerr << "Не удалось открыть файл" << std::endl;

    }
    // файл с мнимой частью
    ifstream imag_file("/home/ivan/Desktop/Work_dir/Yadro/ofdm/imag_part.txt");
    if (!imag_file.is_open()) {
        std::cerr << "Не удалось открыть файл" << std::endl;

    }



    vector <double> real_data;
    vector <double> imag_data;

    // istream_iterator<double> it_real(real_file);
    // istream_iterator<double> it_imag(imag_file);
    // copy(it_real, istream_iterator<double>(), back_inserter(real_data));
    // copy(it_imag, istream_iterator<double>(), back_inserter(imag_data));


    QVector<complex<double>> rx_data;

    double real, imag;
    while (real_file >> real && imag_file >> imag) {
        rx_data.push_back(complex<double>(real, imag));
    }

    // Закрыть файлы
    real_file.close();
    imag_file.close();

    return rx_data;
}


vector<double> convolve(QVector<complex<double>>x, QVector<complex<double>>h) {

    vector<complex<double>> x1 = x.toStdVector();
    vector<complex<double>> h1 = h.toStdVector();

    int n = x1.size() + h1.size() - 1;

    cout << "pss | ofdm "  << h1.size() << " " << x1.size() << endl;

    vector<complex<double>> y(n);

    int size  = static_cast<int>( h1.size());

    x1.insert(x1.begin(), h1.size()-1, complex<double>(0, 0));

    cout << y.size() << endl;
    cout << x1.size() << endl;


    for (int i = 0; i < n; i++) {
        for (int j = 0; j < size; j++) {
            y[i] += x1[i+j] * h1[j];
        }
    }

    vector <double> abs_y(n);

    int count = 0;
    for (int i = 0; i < n; i++){
        abs_y[i] = abs(y[i]);
        if (abs_y[i]> 7000){
            count++;
        }
    }
    cout << "conv1 = " << count << endl;

    return abs_y;
}




MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->widget->xAxis->setRange(0,5887);
    ui->widget->yAxis->setRange(0,15000);

    QVector<complex<double>> pss = pss_on_carrier(128);
    QVector<complex<double>> rx = rx_data();

    for(complex<double> num:rx){
       // cout << num <<  ' ';
    }

    vector <double> convol = convolve(rx, pss);


    cout << "size convol " << convol.size()<< endl;

    for(double num:convol){
       //cout << num <<  ' ';
    }
    QVector<double> qv = QVector<double>::fromStdVector(convol);
    int count = 0;
    for(double num:convol){
       //cout << num <<  ' ';
       if (num > 7000.0){
           count ++;
       }
    }
    cout << endl;
    cout << "conv 1 " <<count << endl;

    QVector<double> x;
    for (int i = 0; i < 5887; i++) {
        x.push_back(i);
    }

    QVector <double> qv2 = conv();


    ui->widget->addGraph();
    ui->widget->graph(0)->addData(x,qv);
    //ui->widget->graph(0)->addData(x,qv2);
    ui->widget->replot();




}

MainWindow::~MainWindow()
{
    delete ui;
}

#include <iostream>
#include <vector>
#include <complex> 
#include <iterator>
#include <fstream>
#include <cmath>

using namespace std;

vector<double> convolve(vector<complex<double>>x, vector<complex<double>>h) {
    
    int n = x.size() + h.size() - 1;
    //cout << h.size() << " " << x.size() << endl;
    vector<complex<double>> y(n);

    x.insert(x.begin(), h.size()-1, complex<double>(0, 0));
        
    // cout << y.size() << endl;
    // cout << x.size() << endl;
    //complex <double> norm_x;
    //complex <double> norm_h;
    double norm_x;
    double norm_h;

    for(int i = 0; i < x.size(); i ++){
        norm_x += abs(x[i]) * abs(x[i]);
    }
    for(int i = 0; i < h.size(); i ++){
        norm_h += abs(h[i]) * abs(h[i]);
    }
    
    vector <double> norm_y;

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < h.size(); j++) {       
            y[i] += x[i+j] * h[j];
        }
        norm_y.push_back((abs(y[i]) / (sqrt(norm_h) * sqrt(norm_x)))*10);
        
    }
    // vector <double> abs_y(n);
    // int count = 0;
    // for (int i = 0; i < n; i++){
    //     abs_y[i] = abs(y[i]);
    //     if (abs_y[i]> 7000){
    //         count++;
    //     }
    // }
    //cout << "conv1 = " << count << endl;


    return norm_y;
}
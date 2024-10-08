#include <QApplication>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <iostream>
#include <vector>
#include <complex>
#include "ofdm_mod.h"

QT_CHARTS_USE_NAMESPACE

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    int N_FFT = 128;
    int N_PILOTS = 6;
    int CP_LEN = 16;

    std::vector<std::complex<double>> PSS(10, std::complex<double>(1.0, -1.0));

    OFDMModulator modulator(N_FFT, N_PILOTS, CP_LEN, PSS);

    // Example data segment
    std::vector<std::vector<std::complex<double>>> data_segments = {
        { {1, 1}, {1, -1}, {1, 1}, {1, -1}, {1, 1}, {1, -1}, {1, 1}, {1, -1},
          {1, 1}, {1, -1}, {1, 1}, {1, -1}, {1, 1}, {1, -1}, {1, 1}, {1, -1} }
    };

    auto ofdm_symbols = modulator.modulate(data_segments);

    // Create a chart
    QChart *chart = new QChart();
    QLineSeries *series = new QLineSeries();

    // Add data to the series
    for (const auto& symbols : ofdm_symbols) {
        for (const auto& symbol : symbols) {
            series->append(symbol.real(), symbol.imag());
        }
    }

    chart->addSeries(series);
    chart->createDefaultAxes();
    chart->setTitle("OFDM Signal Spectrum");

    // Create a chart view and set it as the central widget
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    QMainWindow window;
    window.setCentralWidget(chartView);
    window.resize(800, 600);
    window.show();

    return app.exec();
}

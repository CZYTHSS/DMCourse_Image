#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QMovie>
#include <QMouseEvent>
#include <QDebug>
#include <QWidget>
#include <QtWidgets>
#include <iostream>
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2\core\core.hpp"
#include <stdio.h>
#include <cstdio>
#include <math.h>
#include <vector>
#include <stdlib.h>
#include <fstream>
#include <algorithm>

#include "dialog.h"
#include "lineardialog.h"
#include "nonlineardialog.h"
#include "histogramdialog.h"
#include "utils.h"

using namespace std;
using namespace cv;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

signals:
    void ResetDialog();

public slots:
    void DisplayImage(Mat img);
    void OpenImage();
    void SaveImage();
    void Clockwise90();
    void Flip();
    void Cut();
    void RotateImage();
    void HSIEditDialog();
    void HueChange(int h);
    void SaturationChange(int s);
    void IdensityChange(int l);
    Mat HSIChange();
    void BGR2HSI(Mat &source, Mat &dst);
    void HSI2BGR(Mat &source, Mat &dst);
    void ColorHalftone();
    void SaveHSIChange();
    void CancelHSIChange();
    void LinearEditDialog();
    Mat ReceivePointsData(Points Ps);

    void NonLinearEditDialog();
    Mat ReceiveParams(Params p);

    void EqualizeHist();
    void SpeciHist();
    void Mapping(ArrayWrapper t);

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
     QImage imdisplay;
     QTimer* Timer;
     Dialog *hsiDialog;
     LinearDialog *linear_dialog;
     NonLinearDialog *nonlinear_dialog;
     HistogramDialog *histogram_dialog;

     int hue;   //range (-180,180)
     int saturation, idensity;    //range (-100,100)
     Mat hsi_img;
     Points linear_ps;
     Params nl_params;

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H

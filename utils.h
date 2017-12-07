#ifndef UTILS_H
#define UTILS_H

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

using namespace std;
using namespace cv;

class Coordinate{
public:
    Mat img;
    void CreateCoordinate(int rows, int cols, int lx, int ly, int stepx, int stepy);
    void DrawLine(Point p1, Point p2);
    void DrawLine(Point p1, Point p2, Scalar c);
    void DrawPoint(Point p1);
    void Display();
    void Display(string name, string annotation = "");
    Point ChangeCo(Point p);
};

int BGR2Gray(int b, int g, int r);

#endif // UTILS_H

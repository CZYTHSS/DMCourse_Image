#include "utils.h"

int BGR2Gray(int b, int g, int r){
    double gray = 0.229 * r + 0.587 * g + 0.114 * b;
    int igray = gray;
    return igray;
}

Point Coordinate::ChangeCo(Point p){
    Point p1(p.x + 100, this->img.rows - 100 - p.y);
    return p1;
}

void Coordinate::CreateCoordinate(int rows, int cols, int lx, int ly, int stepx, int stepy){
    Mat img(rows, cols, 16, Scalar(255,255,255));
    if(lx + 100 >= cols || ly + 100 >= rows) {
        qDebug()<<"image is too small, increase rows or cols.";
        return;
    }
    line(img, Point(100, rows - 100), Point(100 + lx, rows - 100), Scalar(0,0,0), 2);
    line(img, Point(100, rows - 100), Point(100, rows - 100 - ly), Scalar(0,0,0), 2);
    for(int i = 100; i <= 100 + lx; i += stepx){
        circle(img, Point(i, rows - 100), 3, Scalar(0,0,0), -1);
        String temp = to_string(i - 100);
        putText(img, temp, Point(i, rows - 80), 1, 1, Scalar(0,0,0));
    }
    for(int i = 0; i <= ly; i += stepy){
        circle(img, Point(100, rows - 100 - i), 3, Scalar(0,0,0), -1);
        String temp = to_string(i);
        putText(img, temp, Point(50, rows - 100 - i), 1, 1, Scalar(0,0,0));
    }
    img.copyTo(this->img);
}

void Coordinate::DrawLine(Point p1, Point p2){
    Point np1 = ChangeCo(p1);
    Point np2 = ChangeCo(p2);
    line(this->img, np1, np2, Scalar(0, 0, 0));
}

void Coordinate::DrawLine(Point p1, Point p2, Scalar c){
    Point np1 = ChangeCo(p1);
    Point np2 = ChangeCo(p2);
    line(this->img, np1, np2, c);
}

void Coordinate::Display(){
    imshow("transfer functions", this->img);
}

void Coordinate::Display(string name, string annotation){
    putText(this->img, annotation, Point(50, 50), 1, 1, Scalar(0,0,0));
    imshow(name, this->img);

}

void Coordinate::DrawPoint(Point p1){
    Point np = ChangeCo(p1);
    circle(this->img, np, 1, Scalar(0,0,0), -1);
}

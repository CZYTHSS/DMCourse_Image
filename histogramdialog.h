#ifndef HISTOGRAMDIALOG_H
#define HISTOGRAMDIALOG_H

#include <QDialog>
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2\core\core.hpp"

#include "utils.h"

using namespace cv;
using namespace std;

enum MAPTYPE:int{
    SML = 1,
    GML = 2,
    REVOKE = 3
};
class ArrayWrapper{
public:
    double h[256];
    MAPTYPE type;
    Mat img;
};

namespace Ui {
class HistogramDialog;
}

class HistogramDialog : public QDialog
{
    Q_OBJECT

signals:
    void sendHistogram(ArrayWrapper transfer);
public slots:
    void OpenImage();
    void SMLMapping();
    void GMLMapping();
    void Revoke();
public:
    explicit HistogramDialog(QWidget *parent = 0);
    ~HistogramDialog();

    Mat ori_image;
    Mat dst_image;

private:
    Ui::HistogramDialog *ui;
};

#endif // HISTOGRAMDIALOG_H

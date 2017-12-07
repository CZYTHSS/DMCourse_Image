#include "mainwindow.h"
#include "ui_mainwindow.h"

double PI = 3.1415926535;

using namespace cv;
//global variables
Mat image;      //主窗体中显示的图像，重要，全局唯一
String address;     //主地址
Mat rotateOrigin;   //开始旋转前的原始图片

//下面的变量都是为裁剪功能服务的
CvPoint prev_pt = { -1,-1 };
CvPoint lu_corner = {-1, -1};
Mat src, inpaint_mask, img0, img, inpainted, mask;
//

enum CUTTYPE:int{
    RECTANGLE = 1,
    CIRCLE = 2
};

struct MouseParams
{
    vector<Point2f> points;
};

void on_mouse(int event, int x, int y, int flags, void* param);
void rec_on_mouse(int event, int x, int y, int flags, void* param);
void cir_on_mouse(int event, int x, int y, int flags, void* param);
void ClickPoints(CUTTYPE c_type);

//opencv and QImage have different channel. opencv is BGR while QImage is RGB, that why when using imshow to show an
//image read by imread, it will work, but we need to use cvtColor to change it when showing it in QImage.
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->hsiDialog = new Dialog(this);
    this->linear_dialog = new LinearDialog(this);
    this->nonlinear_dialog = new NonLinearDialog(this);
    this->histogram_dialog = new HistogramDialog(this);
//    setMouseTracking(true);
    //Mat img;
    address = "C:/Users/Mark Chen/Documents/QtProject/ImageProcessing/img/1.bmp";
    image = imread(address);
    DisplayImage(image);
    connect(ui->openButton, SIGNAL(clicked(bool)), this, SLOT(OpenImage()));
    connect(ui->saveButton, SIGNAL(clicked(bool)), this, SLOT(SaveImage()));
    connect(ui->spin90, SIGNAL(clicked(bool)), this, SLOT(Clockwise90()));
    connect(ui->flipButton, SIGNAL(clicked(bool)), this, SLOT(Flip()));
    connect(ui->recCut, SIGNAL(clicked(bool)), this, SLOT(Cut()));
    connect(ui->rotateButton, SIGNAL(clicked(bool)), this, SLOT(RotateImage()));
    connect(ui->HSI_Space, SIGNAL(triggered(bool)), this, SLOT(HSIEditDialog()));
    connect(ui->Halftone, SIGNAL(triggered(bool)), this, SLOT(ColorHalftone()));
    connect(ui->LinearButton, SIGNAL(triggered(bool)), this, SLOT(LinearEditDialog()));
    connect(ui->NonLinearButton, SIGNAL(triggered(bool)), this, SLOT(NonLinearEditDialog()));
    connect(ui->EqualHist, SIGNAL(triggered(bool)), this, SLOT(EqualizeHist()));
    connect(ui->SpeciHis, SIGNAL(triggered(bool)), this, SLOT(SpeciHist()));
//    Timer = new QTimer(this);
//    connect(Timer, SIGNAL(timeout()), this, SLOT(DisplayImage()));
//    Timer->start();

}
MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::SpeciHist(){
    image.copyTo(this->histogram_dialog->ori_image);
    this->histogram_dialog->exec();
}


void MainWindow::Mapping(ArrayWrapper t){
    if(t.type == REVOKE){
        t.img.copyTo(image);
        DisplayImage(image);
        return;
    }

    //灰度版本
    double src_histogram[256];
    fill(src_histogram, src_histogram + 256, 0);

    for(int i = 0; i < image.rows; i++){
        for(int j = 0; j < image.cols; j++){
            int b = image.at<Vec3b>(i, j)[0];
            int g = image.at<Vec3b>(i, j)[1];
            int r = image.at<Vec3b>(i, j)[2];
            int gray = BGR2Gray(b,g,r);
            src_histogram[gray] += 1;
        }
    }
    //求得累加函数
    for(int m = 0; m < 256 - 1; m++){
        src_histogram[m + 1] += src_histogram[m];
    }
    //归一化
    for(int m = 0; m < 256; m++){
        src_histogram[m] = src_histogram[m]/(image.rows * image.cols);
    }
    Coordinate co;
    co.CreateCoordinate(600, 600, 400, 400, 50, 50);
    Point last;
    for(int i = 0; i <= 255; i++){
        double x, y;
        x = i;
        y = 255.0 * src_histogram[i];
        if(i == 0) {
            last.x = x;
            last.y = y;
        }
        else {
            co.DrawLine(last, Point(x,y));
            last.x = x;
            last.y = y;
        }

    }
    for(int i = 0; i <= 255; i++){
        double x, y;
        x = i;
        y = 255.0 * t.h[i];
        if(i == 0) {
            last.x = x;
            last.y = y;
        }
        else {
            co.DrawLine(last, Point(x,y), Scalar(255, 0, 0));
            last.x = x;
            last.y = y;
        }

    }
    co.Display("Histogram", "blue -- destination image, black -- source image");

    //做映射操作到新的函数值上

    //case SML
    //z = G^(-1)(T(r))
    if(t.type == SML){
        for(int i = 0; i < image.rows; i++){
            for(int j = 0; j < image.cols; j++){
                int b = image.at<Vec3b>(i, j)[0];
                int g = image.at<Vec3b>(i, j)[1];
                int r = image.at<Vec3b>(i, j)[2];
                int gray = BGR2Gray(b, g, r);
                double tr = src_histogram[gray];
                double diff = 1;
                int index = 0;
                for(int m = 0; m < 256; m++){
                    double temp = abs(tr - t.h[m]);
                    if(temp < diff) {
                        diff = temp;
                        index = m;
                    }
                }
                double ratio = double(index) / double(gray);
                for(int n = 0; n < 3; n++){
                    unsigned int var = image.at<Vec3b>(i,j)[n];
                    unsigned int new_var = var * ratio;
                    new_var = (new_var > 255) ? 255 : new_var;
                    new_var = (new_var < 0) ? 0 : new_var;
                    image.at<Vec3b>(i, j)[n] = new_var;
                }
            }
        }
    }
    else if(t.type == GML){
        int mapping[256];
        int index_s = 0;    //source index
        int index_d = 0;    //destination index
        while(index_s < 256 && index_d < 256){
            while(abs(src_histogram[index_s+1] - t.h[index_d]) < abs(src_histogram[index_s] - t.h[index_d])){
                mapping[index_s] = index_d;
                index_s += 1;
            }
            mapping[index_s] = index_d;
            index_d++;
            index_s++;
        }
        for(int i = 0; i < image.rows; i++){
            for(int j = 0; j < image.cols; j++){
                int b = image.at<Vec3b>(i, j)[0];
                int g = image.at<Vec3b>(i, j)[1];
                int r = image.at<Vec3b>(i, j)[2];
                int gray = BGR2Gray(b, g, r);
                double new_gray = mapping[gray];
                double ratio = new_gray / double(gray);
                for(int n = 0; n < 3; n++){
                    unsigned int var = image.at<Vec3b>(i,j)[n];
                    unsigned int new_var = var * ratio;
                    new_var = (new_var > 255) ? 255 : new_var;
                    new_var = (new_var < 0) ? 0 : new_var;
                    image.at<Vec3b>(i, j)[n] = new_var;
                }
            }
        }
    }
    DisplayImage(image);
}

void MainWindow::EqualizeHist(){
    //灰度版本
    Mat img(image.size(), image.type());
    BGR2HSI(image, img);
    double histogram[256];
    fill(histogram, histogram + 256, 0);

    for(int i = 0; i < img.rows; i++){
        for(int j = 0; j < img.cols; j++){
            int idensity = img.at<Vec3b>(i, j)[2];
            histogram[idensity] += 1;
        }
    }
    //求得累加函数
    for(int m = 0; m < 256 - 1; m++){
        histogram[m + 1] += histogram[m];
    }
    //归一化
    for(int m = 0; m < 256; m++){
        histogram[m] = histogram[m]/(img.rows * img.cols);
    }
    //做映射操作到新的函数值上
    for(int i = 0; i < img.rows; i++){
        for(int j = 0; j < img.cols; j++){
            int idensity = img.at<Vec3b>(i, j)[2];
            unsigned int new_var = round(255.0 * histogram[idensity]);
            new_var = (new_var > 255) ? 255 : new_var;
            new_var = (new_var < 0) ? 0 : new_var;
            img.at<Vec3b>(i, j)[2] = new_var;
        }
    }
    HSI2BGR(img, image);
    Coordinate co;
    co.CreateCoordinate(600, 600, 400, 400, 50, 50);
    Point last;
    for(int i = 0; i <= 255; i++){
        double x, y;
        x = i;
        y = 255.0 * histogram[i];
        if(i == 0) {
            last.x = x;
            last.y = y;
        }
        else {
            co.DrawLine(last, Point(x,y));
            last.x = x;
            last.y = y;
        }

    }
    co.Display();
    DisplayImage(image);

    //分颜色通道版本
    /*
    double histogram[256];
    fill(histogram, histogram + 256, 0);
    for(int n = 0; n < 3; n++){
        for(int i = 0; i < image.rows; i++){
            for(int j = 0; j < image.cols; j++){
                int var = image.at<Vec3b>(i, j)[n];
                histogram[var] += 1;
            }
        }
        //求得累加函数
        for(int m = 0; m < 256 - 1; m++){
            histogram[m + 1] += histogram[m];
        }
        //归一化
        for(int m = 0; m < 256; m++){
            histogram[m] = histogram[m]/(image.rows * image.cols);
        }
        //做映射操作到新的函数值上
        for(int i = 0; i < image.rows; i++){
            for(int j = 0; j < image.cols; j++){
                int var = image.at<Vec3b>(i, j)[n];
                double new_var = 255.0 * histogram[var];
                new_var = (new_var > 255) ? 255 : new_var;
                new_var = (new_var < 0) ? 0 : new_var;
                image.at<Vec3b>(i, j)[n] = new_var;
            }
        }
        fill(histogram, histogram + 256, 0);
    }
    Coordinate co;
    co.CreateCoordinate(600, 600, 400, 400, 50, 50);
    Point last;
    for(int i = 0; i <= 255; i++){
        double x, y;
        x = i;
        y = 255.0 * histogram[i];
        if(i == 0) {
            last.x = x;
            last.y = y;
        }
        else {
            co.DrawLine(last, Point(x,y));
            last.x = x;
            last.y = y;
        }

    }
    co.Display();
    DisplayImage(image);
    */
}


Mat MainWindow::ReceiveParams(Params p){
    Mat img(image.size(), image.type());
    if(p.type == LOG){
        this->nl_params.log_a = p.log_a;
        this->nl_params.log_b = p.log_b;
        this->nl_params.log_c = p.log_c;
        this->nl_params.type = p.type;
        for(int i = 0; i < image.rows; i++){
            for(int j = 0; j < image.cols; j++){
                for(int n = 0; n < 3; n++){
                    unsigned int new_var = int(p.log_a + log(image.at<Vec3b>(i, j)[n]  + 1)/(p.log_b * log(p.log_c)));
                    new_var = (new_var > 255) ? 255 : new_var;
                    new_var = (new_var < 0) ? 0 : new_var;
                    img.at<Vec3b>(i, j)[n] = new_var;
                }
            }
        }
    }
    else if(p.type == EXP){
        this->nl_params.exp_a = p.exp_a;
        this->nl_params.exp_b = p.exp_b;
        this->nl_params.exp_c = p.exp_c;
        this->nl_params.type = p.type;
        for(int i = 0; i < image.rows; i++){
            for(int j = 0; j < image.cols; j++){
                for(int n = 0; n < 3; n++){
                    unsigned int new_var = int(pow(p.exp_b, p.exp_c * (image.at<Vec3b>(i, j)[n] - p.exp_a)) - 1);
                    new_var = (new_var > 255) ? 255 : new_var;
                    new_var = (new_var < 0) ? 0 : new_var;
                    img.at<Vec3b>(i, j)[n] = new_var;
                }
            }
        }
    }
    else if(p.type == GAMMA){
        this->nl_params.g_c = p.g_c;
        this->nl_params.g_gamma = p.g_gamma;
        this->nl_params.type = p.type;
        for(int i = 0; i < image.rows; i++){
            for(int j = 0; j < image.cols; j++){
                for(int n = 0; n < 3; n++){
                    unsigned int new_var = int(p.g_c * pow(image.at<Vec3b>(i, j)[n], p.g_gamma));
                    new_var = (new_var > 255) ? 255 : new_var;
                    new_var = (new_var < 0) ? 0 : new_var;
                    img.at<Vec3b>(i, j)[n] = new_var;
                }
            }
        }
    }
    DisplayImage(img);
    return img;
}

void MainWindow::NonLinearEditDialog(){
    this->nonlinear_dialog->exec();
    Mat img = ReceiveParams(this->nl_params);
    img.copyTo(image);
    DisplayImage(image);
}

void MainWindow::LinearEditDialog(){
    this->linear_dialog->exec();
    Mat img = ReceivePointsData(this->linear_ps);
    img.copyTo(image);
    DisplayImage(image);
}

Mat MainWindow::ReceivePointsData(Points Ps){
    this->linear_ps = Ps;
    Mat img(image.size(), image.type());
    double x1,x2,y1,y2;
    x1 = Ps.x1;
    x2 = Ps.x2;
    y1 = Ps.y1;
    y2 = Ps.y2;
    //l1: y = (y1/x1) * x;
    //l2: y = (y2 - y1)/(x2 - x1) * (x - x1) + y1
    //l3: y = [(255 - y2)/(255 - x2)] * (x - x2) + y2
    //设三段斜率为s1, s2, s3;
    double s1, s2, s3;
    s1 = y1/x1;
    s2 = (y2 - y1)/(x2 - x1);
    s3 = (255 - y2)/(255 - x2);
    for(int i = 0; i < image.rows; i++){
        for(int j = 0; j < image.cols; j++){
            for(int n = 0; n < 3; n++){
                unsigned int var = image.at<Vec3b>(i, j)[n];
                if(var < x1){
                    unsigned int new_var = s1 * var;
                    new_var = (new_var > 255) ? 255 : new_var;
                    new_var = (new_var < 0) ? 0 : new_var;
                    img.at<Vec3b>(i, j)[n] = new_var;
                }
                else if(var <=x2){
                    unsigned int new_var = s2 * (var - x1) + y1;
                    new_var = (new_var > 255) ? 255 : new_var;
                    new_var = (new_var < 0) ? 0 : new_var;
                    img.at<Vec3b>(i, j)[n] = new_var;
                }
                else{
                    unsigned int new_var = s3 * (var - x2) + y2;
                    new_var = (new_var > 255) ? 255 : new_var;
                    new_var = (new_var < 0) ? 0 : new_var;
                    img.at<Vec3b>(i, j)[n] = new_var;
                }
            }
        }
    }
    DisplayImage(img);
    return img;
}

void MainWindow::ColorHalftone(){
    Mat img(image.rows, image.cols, image.type(), Scalar(255,255,255));
    double a = 12;  //中心坐标点的间隔
    double rmax = a/2;  //圆的最大可能半径。是坐标间隔的一半
    double rmin = rmax/3;  //圆最小可能半径
    //red channel. vertical
    for(int i = 0; i < image.rows; i++){
        for(int j = 0; j < image.cols; j++){
            if(int(i - rmax) % int(a) == 0 && int(j - rmax) % int(a) == 0){
                double red = image.at<Vec3b>(i, j)[2];
                double ratio = red / 255.0;
                ratio = (ratio - 0.33) < 0 ? 0 : (ratio - 0.33);
                int radius = (rmax - rmin) * (1 - ratio) + rmin;
                for(int m = i - rmax; m < i + rmax; m++){
                    for(int n = j - rmax; n < j + rmax; n++){
                        if(m > 0 && n > 0 && m < image.rows && n < image.cols){
                            if(pow(m - i,2) + pow(n - j, 2) < pow(radius, 2))
                                img.at<Vec3b>(m, n)[2] = image.at<Vec3b>(m, n)[2];
                        }
                    }
                }
                //circle(img, Point(i, j), radius, Scalar(0, 0, red), -1);
            }
        }
    }

    //green channel.
    double slope = 0.5; //l1: slope * x - y = 0; l2: x + slope * y = 0
    for(int i = 0; i < image.rows; i++){
        for(int j = 0; j < image.cols; j++){
            double d1, d2;
            d1 = abs(slope*i - j)/sqrt(pow(slope,2) + 1);
            d2 = abs(i + slope * j)/sqrt(pow(slope,2) + 1);
            if((int(ceil(d1)) % int(a) == 0 || int(floor(d1)) % int(a) == 0) && (int(ceil(d2)) % int(a) == 0 || int(floor(d2)) % int(a) == 0)){
                double green = image.at<Vec3b>(i, j)[1];
                double ratio = green / 255.0;
                ratio = (ratio - 0.33) < 0 ? 0 : (ratio - 0.33);
                int radius = (rmax - rmin) * (1 - ratio) + rmin;
                for(int m = i - rmax; m < i + rmax; m++){
                    for(int n = j - rmax; n < j + rmax; n++){
                        if(m > 0 && n > 0 && m < image.rows && n < image.cols){
                            if(pow(m - i,2) + pow(n - j, 2) < pow(radius, 2))
                                img.at<Vec3b>(m, n)[1] = image.at<Vec3b>(m, n)[1];
                        }
                    }
                }
            }
        }
    }

    //blue channel
    //slope = 1 / slope;
    for(int i = 0; i < image.rows; i++){
        for(int j = 0; j < image.cols; j++){\
            double d1, d2;
            d1 = abs(slope*i - j)/sqrt(pow(slope,2) + 1);
            d2 = abs(i + slope * j)/sqrt(pow(slope,2) + 1);
            if((int(ceil(d1)) % int(a) == 0 || int(floor(d1)) % int(a) == 0) && (int(ceil(d2)) % int(a) == 0 || int(floor(d2)) % int(a) == 0)){
                int temp = j;
                j = image.cols - j - 1;
                double blue = image.at<Vec3b>(i, j)[0];
                double ratio = blue / 255.0;
                ratio = (ratio - 0.33) < 0 ? 0 : (ratio - 0.33);
                int radius = (rmax - rmin) * (1 - ratio) + rmin;
                for(int m = i - rmax; m < i + rmax; m++){
                    for(int n = j - rmax; n < j + rmax; n++){
                        if(m > 0 && n > 0 && m < image.rows && n < image.cols){
                            if(pow(m - i,2) + pow(n - j, 2) < pow(radius, 2))
                                img.at<Vec3b>(m, n)[0] = image.at<Vec3b>(m, n)[0];
                        }
                    }
                }
                j = temp;
            }
        }
    }

    img.copyTo(image);
    DisplayImage(image);
}

void MainWindow::BGR2HSI(Mat &source, Mat &dst){
    double h, s, l;    //hue~(0,PI), saturation~(0,1),idensity(0,255)
    for(int i=0;i<source.rows;i++)
    {
       for(int j=0;j<source.cols;j++)
       {
           double b,g,r;
           b = source.at<Vec3b>(i,j)[0];
           g = source.at<Vec3b>(i,j)[1];
           r = source.at<Vec3b>(i,j)[2];
           
           b = b / 255;
           g = g / 255;
           r = r / 255;
           double maxnum = max(max(b,g), max(b,r));
           double minnum = min(min(b,g), min(b,r));

           
           if(maxnum == minnum)h = 0;
           else if(maxnum == r && g >= b) h = 60 * (g - b)/(maxnum - minnum);
           else if(maxnum == r && g < b) h = 60 * (g - b)/(maxnum - minnum) + 360;
           else if(maxnum == g) h = 60 * (b - r)/(maxnum - minnum) + 120;
           else if(maxnum == b) h = 60 * (r - g)/(maxnum - minnum) + 240;
           
           l = 0.5 * (maxnum + minnum);
           
           if(l == 0 || maxnum == minnum) s = 0;
           else if(l > 0 && l <= 0.5) s = (maxnum - minnum)/ (2 * l);
           else if(l > 0.5) s = (maxnum - minnum)/(2 - 2 * l);

           dst.at<Vec3b>(i, j)[0] = 255 * h/360;
           dst.at<Vec3b>(i, j)[1] = s * 255;
           dst.at<Vec3b>(i, j)[2] = l * 255;
       }
    }
}

void MainWindow::HSI2BGR(Mat &source, Mat &dst){
    double b, g, r;
    double h, s, l;
    for(int i=0;i<source.rows;i++)
    {
       for(int j=0;j<source.cols;j++)
       {
           h = 360 * (source.at<Vec3b>(i,j)[0] / 255.0);    //range (0, 360)
           s = source.at<Vec3b>(i,j)[1] / 255.0;            //range (0,1)
           l = source.at<Vec3b>(i,j)[2] / 255.0;              //range (0,1)

           if(s == 0){
               b = l * 255;
               r = b;
               g = b;
           }
           else{
               double q,p;
               if(l < 0.5){
                   q = l * (1 + s);
               }
               else if(l >= 0.5){
                   q = l + s - (l * s);
               }
               p = 2 * l - q;
               double hk = h / 360;
               double tr = hk + 1.0/3.0;
               double tg = hk;
               double tb = hk - 1.0/3.0;
               tr = (tr < 0) ? (tr + 1) : tr;
               tr = (tr > 1) ? (tr - 1) : tr;
               tg = (tg < 0) ? (tg + 1) : tg;
               tg = (tg > 1) ? (tg - 1) : tg;
               tb = (tb < 0) ? (tb + 1) : tb;
               tb = (tb > 1) ? (tb - 1) : tb;
               double color[3];
               double tc[3];
               tc[0] = tr;
               tc[1] = tg;
               tc[2] = tb;
               for(int m = 0; m < 3; m++){
                   if(tc[m] < 1.0/6.0) color[m] = p + ((q - p) * 6 * tc[m]);
                   else if(tc[m] < 0.5) color[m] = q;
                   else if(tc[m] < 2.0/3.0) color[m] = p + ((q - p) * 6 * (2.0/3.0 - tc[m]));
                   else color[m] = p;
               }
               r = color[0] * 255;
               g = color[1] * 255;
               b = color[2] * 255;
           }
           dst.at<Vec3b>(i, j)[0] = b;
           dst.at<Vec3b>(i, j)[1] = g;
           dst.at<Vec3b>(i, j)[2] = r;
       }
    }
}

Mat MainWindow::HSIChange(){
    double h, s, l;
    h = this->hue;
    s = this->saturation;
    l = this->idensity;
    Mat temp;
    this->hsi_img.copyTo(temp);
    if(h >= -180 && h <= 180){
        for(int i=0;i<temp.rows;i++)
        {
           for(int j=0;j<temp.cols;j++)
           {
               double oldhue = 360 * (temp.at<Vec3b>(i, j)[0] / 255.0);
               double diff = h;
               double newHue = oldhue + diff;
               if(newHue > 360)
                   newHue -= 360;
               else if(newHue < 0)
                   newHue += 360;
               int t = (newHue / 360) * 255;
               temp.at<Vec3b>(i, j)[0] = t;
           }
        }
    }
    if(s >= -100 && s <= 100){
        for(int i=0;i<temp.rows;i++)
        {
           for(int j=0;j<temp.cols;j++)
           {
               double oldS = 100 * (temp.at<Vec3b>(i, j)[1] / 255.0);
               double diff = s;
               double newS = oldS + diff;
               if(newS > 100)
                   newS = 100;
               else if(newS < 0)
                   newS = 0;
               int t = (newS / 100) * 255;
               temp.at<Vec3b>(i, j)[1] = t;
           }
        }
    }
    if(l >= -100 && l <= 100){
        for(int i=0;i<temp.rows;i++)
        {
           for(int j=0;j<temp.cols;j++)
           {
               double oldI = 100 * (temp.at<Vec3b>(i, j)[2] / 255.0);
               double diff = l;
               double newI = oldI + diff;
               if(newI > 100)
                   newI = 100;
               else if(newI < 0)
                   newI = 0;
               int t = (newI / 100) * 255;
               temp.at<Vec3b>(i, j)[2] = t;
           }
        }
    }
    Mat img;
    img.create(temp.rows, temp.cols, temp.type());
    HSI2BGR(temp, img);
    DisplayImage(img);
    return(img);
}

void MainWindow::HueChange(int h){
    this->hue = h;
    HSIChange();
}

void MainWindow::SaturationChange(int s){
    this->saturation = s;
    HSIChange();
}

void MainWindow::IdensityChange(int idens){
    this->idensity = idens;
    HSIChange();
}

void MainWindow::SaveHSIChange(){
    Mat temp = HSIChange();
    temp.copyTo(image);
}

void MainWindow::CancelHSIChange(){
    DisplayImage(image);
}

void MainWindow::HSIEditDialog(){
    //初始化hsi，每次重新编辑时hsi都从0开始
    this->hue = 0;
    this->saturation = 0;
    this->idensity = 0;
    int cols = image.cols;
    int rows = image.rows;
    this->hsi_img.create(rows, cols, image.type());
    BGR2HSI(image, this->hsi_img);
    emit ResetDialog();

    this->hsiDialog->exec();
}

//input an opencv default format image, cvt it and show.
void MainWindow::DisplayImage(Mat img){
    Mat img2;
    cvtColor(img, img2, CV_BGR2RGB);
    QImage imdisplay((uchar*)img2.data, img2.cols, img2.rows, img2.step, QImage::Format_RGB888);
    ui->display_image->setPixmap(QPixmap::fromImage(imdisplay));
}

void MainWindow::SaveImage(){
    QString fileName = QFileDialog::getSaveFileName(this,tr("Save Image"),".",tr("Image File(*.png *.jpg *.jpeg *.bmp)"));
    if(fileName.toStdString() == ""){
        qDebug()<<"save image canceled";
    }
    else cv::imwrite(fileName.toStdString(), image);
}

void MainWindow::OpenImage(){
    QString fileName = QFileDialog::getOpenFileName(this,tr("Open Image"),".",tr("Image File(*.png *.jpg *.jpeg *.bmp)"));
    if(fileName.toStdString() == ""){
        qDebug()<<"open image canceled";
    }
    else image = cv::imread(fileName.toStdString());
    DisplayImage(image);
}

void MainWindow::Clockwise90(){
    int cols = image.cols;
    int rows = image.rows;
    Mat img;
    img.create(cols, rows, image.type());
    for(int i=0;i<image.rows;i++)
    {
       for(int j=0;j<image.cols;j++)
       {
           img.at<Vec3b>(j,rows - i - 1)[0]=image.at<Vec3b>(i,j)[0];
           img.at<Vec3b>(j,rows - i - 1)[1]=image.at<Vec3b>(i,j)[1];
           img.at<Vec3b>(j,rows - i - 1)[2]=image.at<Vec3b>(i,j)[2];
       }
    }
    img.copyTo(image);
    DisplayImage(image);
}

void MainWindow::Flip(){
    int cols = image.cols;
    int rows = image.rows;
    Mat img;
    img.create(rows, cols, image.type());
    for(int i=0;i<image.rows;i++)
    {
       for(int j=0;j<image.cols;j++)
       {
           img.at<Vec3b>(rows - i - 1,j)[0]=image.at<Vec3b>(i,j)[0];
           img.at<Vec3b>(rows - i - 1,j)[1]=image.at<Vec3b>(i,j)[1];
           img.at<Vec3b>(rows - i - 1,j)[2]=image.at<Vec3b>(i,j)[2];
       }
    }
    img.copyTo(image);
    DisplayImage(image);
}

void MainWindow::Cut(){
    if(ui->RecRadioButton->isChecked()){
        ClickPoints(RECTANGLE);
        DisplayImage(image);
    }
    else if(ui->CirRadioButton->isChecked()){
        ClickPoints(CIRCLE);
        DisplayImage(image);
    }
    else qDebug()<<"not checked";
}

void MainWindow::RotateImage(){
    QString qstr_angle = ui->angleEdit->text();
    String str_angle = qstr_angle.toStdString();
    if(str_angle == ""){
        qDebug()<<"rotation angle is not input.";
    }
    else{
        double d_angle = stod(str_angle);
        qDebug()<<d_angle;
    //    double angle = 30;
        Point2f src_center(image.cols/2.0F, image.rows/2.0F);
        Mat rot_mat = getRotationMatrix2D(src_center, d_angle, 1.0);
        Mat dst;
        Mat temp;
        image.copyTo(temp);
        warpAffine(temp, dst, rot_mat, image.size());
        dst.copyTo(image);
        DisplayImage(image);
    }
}

void on_mouse(int event, int x, int y, int flags, void* param){
    MouseParams* mp = (MouseParams*)param;
    CvFont font;
    cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.5, 0.5, 0, 1, CV_AA);//字体结构初始化

    if ((event == CV_EVENT_LBUTTONDOWN) && (flags))//鼠标左键按下事件发生
    {
        CvPoint pt = cvPoint(x, y);//获取当前点的横纵坐标值
        Point2f P(pt.x, pt.y);
        mp->points.push_back(P);
        String temp = "(" + to_string(pt.x) + "," + to_string(pt.y) + ")";

        putText(img,temp,pt,1,1,Scalar(255,255,255));
        circle(img, pt, 2, Scalar(255,0,0), FILLED, CV_AA, 0);
        imshow("src", img);
    }
}

void rec_on_mouse(int event, int x, int y, int flags, void* param){
    MouseParams* mp = (MouseParams*)param;

    if ((event == CV_EVENT_LBUTTONDOWN) && (flags))//鼠标左键按下事件发生
    {
        CvPoint pt = cvPoint(x, y);//获取当前点的横纵坐标值
        lu_corner = pt;
        Point2f P(pt.x, pt.y);
        mp->points.push_back(P);
        String temp = "(" + to_string(pt.x) + "," + to_string(pt.y) + ")";
        putText(img,temp,pt,1,1,Scalar(255,255,255,0));
        circle(img, pt, 2, Scalar(255,0,0,0), FILLED, CV_AA, 0);
        imshow("src", img);
    }
    else if(event == CV_EVENT_LBUTTONUP){
        CvPoint pt = cvPoint(x, y);//获取当前点的横纵坐标值
        CvPoint p1, p2, p3, p4;
        p1 = lu_corner;
        p2 = cvPoint(lu_corner.x, pt.y);
        p3 = cvPoint(pt.x, lu_corner.y);
        p4 = pt;
        Mat temp;
        temp.create(abs(p4.y - p1.y) + 1, abs(p4.x - p1.x) + 1, img.type());
        int srow = p4.y > p1.y ? p1.y : p4.y;
        int erow = p4.y <= p1.y ? p1.y : p4.y;
        int scol = p4.x > p1.x ? p1.x : p4.x;
        int ecol = p4.x <= p1.x ? p1.x : p4.x;

        int r = 0, c = 0;
        for(int i=srow;i<=erow;i++)
        {
           for(int j=scol;j<ecol;j++)
           {
               temp.at<Vec3b>(r, c)[0]=src.at<Vec3b>(i,j)[0];
               temp.at<Vec3b>(r, c)[1]=src.at<Vec3b>(i,j)[1];
               temp.at<Vec3b>(r, c)[2]=src.at<Vec3b>(i,j)[2];
               c++;
           }
           r++;
           c = 0;
        }
        r = 0;
        c = 0;

        temp.copyTo(image);
        imshow("src", image);
    }
    else if ((event == CV_EVENT_MOUSEMOVE) && (flags & CV_EVENT_FLAG_LBUTTON)){
        src.copyTo(img);
        String temp = "(" + to_string(lu_corner.x) + "," + to_string(lu_corner.y) + ")";
        putText(img,temp,lu_corner,1,1,Scalar(255,255,255));
        circle(img, lu_corner, 2, Scalar(255,0,0), FILLED, CV_AA, 0);

        CvPoint pt = cvPoint(x, y);
        CvPoint p1, p2, p3, p4;
        p1 = lu_corner;
        p2 = cvPoint(lu_corner.x, pt.y);
        p3 = cvPoint(pt.x, lu_corner.y);
        p4 = pt;
        line(img, p1, p2, cvScalarAll(255), 2, 8, 0);
        line(img, p2, p4, cvScalarAll(255), 2, 8, 0);
        line(img, p4, p3, cvScalarAll(255), 2, 8, 0);
        line(img, p1, p3, cvScalarAll(255), 2, 8, 0);

        imshow("src", img);

    }

}

void cir_on_mouse(int event, int x, int y, int flags, void* param){
    MouseParams* mp = (MouseParams*)param;

    if ((event == CV_EVENT_LBUTTONDOWN) && (flags))//鼠标左键按下事件发生
    {
        CvPoint pt = cvPoint(x, y);//获取当前点的横纵坐标值
        lu_corner = pt;
        Point2f P(pt.x, pt.y);
        mp->points.push_back(P);
        String temp = "(" + to_string(pt.x) + "," + to_string(pt.y) + ")";
        putText(img,temp,pt,1,1,Scalar(255,255,255));
        circle(img, pt, 2, Scalar(255,0,0), FILLED, CV_AA, 0);
        imshow("src", img);
    }
    else if(event == CV_EVENT_LBUTTONUP){
        CvPoint pt = cvPoint(x, y);//获取当前点的横纵坐标值
        CvPoint p1, p2, p3, p4;
        p1 = lu_corner;
        p2 = cvPoint(lu_corner.x, pt.y);
        p3 = cvPoint(pt.x, lu_corner.y);
        p4 = pt;

        CvPoint center;
        center = cvPoint((lu_corner.x + pt.x)/2, (lu_corner.y + pt.y)/2);
        int radius = sqrt(pow(lu_corner.x - pt.x, 2) + pow(lu_corner.y - pt.y, 2))/2;
        circle(inpaint_mask, center, radius, Scalar(1, 1, 1));
        floodFill(inpaint_mask, Point((lu_corner.x + x)/2, (lu_corner.y + y)/2), Scalar(1,1,1));

        Mat temp;
        temp = src.mul(inpaint_mask);
        temp.copyTo(image);
        imshow("src", image);

        Mat t(img.size(), img.type(), Scalar(0,0,0));
        t.copyTo(inpaint_mask);
    }
    else if ((event == CV_EVENT_MOUSEMOVE) && (flags & CV_EVENT_FLAG_LBUTTON)){
        src.copyTo(img);
        String temp = "(" + to_string(lu_corner.x) + "," + to_string(lu_corner.y) + ")";
        putText(img,temp,lu_corner,1,1,Scalar(255,255,255));
        circle(img, lu_corner, 2, Scalar(255,0,0), FILLED, CV_AA, 0);

        CvPoint pt = cvPoint(x, y);
        CvPoint center;
        center = cvPoint((lu_corner.x + pt.x)/2, (lu_corner.y + pt.y)/2);
        int radius = sqrt(pow(lu_corner.x - pt.x, 2) + pow(lu_corner.y - pt.y, 2))/2;

        circle(img, center, radius, Scalar(255, 255, 255));


        imshow("src", img);

    }

}

void ClickPoints(CUTTYPE c_type)
{
    MouseParams mp;

    //opencv3 rewrite
    image.copyTo(src);
    src.copyTo(img);
    Mat t(img.size(), img.type(), Scalar(0,0,0));
    t.copyTo(inpaint_mask);

    //opencv3 rewrite
    namedWindow("src", WINDOW_AUTOSIZE);
    if(c_type == RECTANGLE){
        setMouseCallback("src", rec_on_mouse, (void*)&mp);
    }
    else if(c_type == CIRCLE){
        setMouseCallback("src", cir_on_mouse, (void*)&mp);
    }
    imshow("src", img);
    waitKey(0);
    destroyAllWindows();
}

#include "histogramdialog.h"
#include "ui_histogramdialog.h"

HistogramDialog::HistogramDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HistogramDialog)
{
    ui->setupUi(this);
    connect(ui->OpenButton, SIGNAL(clicked(bool)), this, SLOT(OpenImage()));
    connect(ui->SMLButton, SIGNAL(clicked(bool)), this, SLOT(SMLMapping()));
    connect(ui->RevokeButton, SIGNAL(clicked(bool)), this, SLOT(Revoke()));
    connect(this, SIGNAL(sendHistogram(ArrayWrapper)), parent, SLOT(Mapping(ArrayWrapper)));
    connect(ui->GMLButton, SIGNAL(clicked(bool)), this, SLOT(GMLMapping()));
}

HistogramDialog::~HistogramDialog()
{
    delete ui;
}

void HistogramDialog::OpenImage(){
    if(this->dst_image.empty()){
        qDebug()<<"Null";
    }
    QString fileName = QFileDialog::getOpenFileName(this,tr("Open Image"),".",tr("Image File(*.png *.jpg *.jpeg *.bmp)"));
    if(fileName.toStdString() == ""){
        qDebug()<<"open image canceled";
    }
    else this->dst_image = cv::imread(fileName.toStdString());
    //imshow("dst", this->dst_image);
}

void HistogramDialog::SMLMapping(){
    if(this->dst_image.empty()){
        QMessageBox::critical(0 ,
        "错误" , "hasn't opened a destination image yet",
        QMessageBox::Ok | QMessageBox::Default ,
        QMessageBox::Cancel | QMessageBox::Escape , 	0 );
    }
    else {
        //灰度版本
        double histogram[256];
        fill(histogram, histogram + 256, 0);

        for(int i = 0; i < this->dst_image.rows; i++){
            for(int j = 0; j < this->dst_image.cols; j++){
                int b = this->dst_image.at<Vec3b>(i, j)[0];
                int g = this->dst_image.at<Vec3b>(i, j)[1];
                int r = this->dst_image.at<Vec3b>(i, j)[2];
                int gray = BGR2Gray(b,g,r);
                histogram[gray] += 1;
            }
        }
        //求得累加函数
        for(int m = 0; m < 256 - 1; m++){
            histogram[m + 1] += histogram[m];
        }
        //归一化
        ArrayWrapper transfer;
        for(int m = 0; m < 256; m++){
            histogram[m] = histogram[m]/(this->dst_image.rows * this->dst_image.cols);
            transfer.h[m] = histogram[m];
        }
        transfer.type = SML;
        emit sendHistogram(transfer);
    }
}

void HistogramDialog::GMLMapping(){
    if(this->dst_image.empty()){
        QMessageBox::critical(0 ,
        "错误" , "hasn't opened a destination image yet",
        QMessageBox::Ok | QMessageBox::Default ,
        QMessageBox::Cancel | QMessageBox::Escape , 	0 );
    }
    else {
        //灰度版本
        double histogram[256];
        fill(histogram, histogram + 256, 0);

        for(int i = 0; i < this->dst_image.rows; i++){
            for(int j = 0; j < this->dst_image.cols; j++){
                int b = this->dst_image.at<Vec3b>(i, j)[0];
                int g = this->dst_image.at<Vec3b>(i, j)[1];
                int r = this->dst_image.at<Vec3b>(i, j)[2];
                int gray = BGR2Gray(b,g,r);
                histogram[gray] += 1;
            }
        }
        //求得累加函数
        for(int m = 0; m < 256 - 1; m++){
            histogram[m + 1] += histogram[m];
        }
        //归一化
        ArrayWrapper transfer;
        for(int m = 0; m < 256; m++){
            histogram[m] = histogram[m]/(this->dst_image.rows * this->dst_image.cols);
            transfer.h[m] = histogram[m];
        }
        transfer.type = GML;
        emit sendHistogram(transfer);
    }
}

void HistogramDialog::Revoke(){
    if(this->dst_image.empty()){
        QMessageBox::critical(0 ,
        "错误" , "you haven't made any change!",
        QMessageBox::Ok | QMessageBox::Default ,
        QMessageBox::Cancel | QMessageBox::Escape , 	0 );
    }
    else {
        ArrayWrapper t;
        t.type = REVOKE;
        this->ori_image.copyTo(t.img);
        emit sendHistogram(t);
    }
}

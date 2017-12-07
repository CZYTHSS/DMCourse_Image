#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    //connect(ui->sliderH, SIGNAL(valueChanged(int)), parent, SLOT(HueChange(int)));
    connect(ui->sliderH, SIGNAL(valueChanged(int)), parent, SLOT(HueChange(int)));
    connect(ui->sliderS, SIGNAL(valueChanged(int)), parent, SLOT(SaturationChange(int)));
    connect(ui->sliderI, SIGNAL(valueChanged(int)), parent, SLOT(IdensityChange(int)));

    //让spinbox和slider之间联动
    connect(ui->sliderH, SIGNAL(valueChanged(int)), ui->spinBoxH, SLOT(setValue(int)));
    connect(ui->sliderS, SIGNAL(valueChanged(int)), ui->spinBoxS, SLOT(setValue(int)));
    connect(ui->sliderI, SIGNAL(valueChanged(int)), ui->spinBoxI, SLOT(setValue(int)));
    connect(ui->spinBoxH, SIGNAL(valueChanged(int)), ui->sliderH, SLOT(setValue(int)));
    connect(ui->spinBoxS, SIGNAL(valueChanged(int)), ui->sliderS, SLOT(setValue(int)));
    connect(ui->spinBoxI, SIGNAL(valueChanged(int)), ui->sliderI, SLOT(setValue(int)));
    connect(ui->buttonBox, SIGNAL(accepted()), parent, SLOT(SaveHSIChange()));
    connect(ui->buttonBox, SIGNAL(rejected()), parent, SLOT(CancelHSIChange()));
    connect(parent, SIGNAL(ResetDialog()), this, SLOT(ResetDialog()));
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::ResetDialog(){
    ui->sliderH->setValue(0);
    ui->sliderI->setValue(0);
    ui->sliderS->setValue(0);
}

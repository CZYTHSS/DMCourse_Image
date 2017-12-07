#include "lineardialog.h"
#include "ui_lineardialog.h"

LinearDialog::LinearDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LinearDialog)
{
    ui->setupUi(this);
    connect(ui->applyButton, SIGNAL(clicked(bool)), this, SLOT(ApplyLinearTrans()));
    connect(this, SIGNAL(sendPointsData(Points)), parent, SLOT(ReceivePointsData(Points)));
    connect(ui->revokeButton, SIGNAL(clicked(bool)), this, SLOT(Revoke()));
}

LinearDialog::~LinearDialog()
{
    delete ui;
}

void LinearDialog::Revoke(){
    Points ps;
    ps.x1 = 0;
    ps.x2 = 255;
    ps.y1 = 0;
    ps.y2 = 255;
    ui->x1Box->setValue(0);
    ui->x2Box->setValue(0);
    ui->y1Box->setValue(0);
    ui->y2Box->setValue(0);
    emit sendPointsData(ps);
}

void LinearDialog::ApplyLinearTrans(){   
    int x1 = ui->x1Box->value();
    int x2 = ui->x2Box->value();
    int y1 = ui->y1Box->value();
    int y2 = ui->y2Box->value();
    if(x1 >= x2){
        QMessageBox::critical(0 ,
        "错误" , "x1必须比x2更小!",
        QMessageBox::Ok | QMessageBox::Default ,
        QMessageBox::Cancel | QMessageBox::Escape , 	0 );
    }
    else{
        Points Ps;
        Ps.x1 = x1;
        Ps.x2 = x2;
        Ps.y1 = y1;
        Ps.y2 = y2;
        Coordinate co;
        co.CreateCoordinate(600, 600, 400, 400, 50, 50);
        co.DrawLine(Point(0,0), Point(Ps.x1, Ps.y1));
        co.DrawLine(Point(Ps.x1, Ps.y1), Point(Ps.x2, Ps.y2));
        co.DrawLine(Point(Ps.x2, Ps.y2), Point(255, 255));
        co.DrawPoint(Point(30,80));
        co.Display();

        emit sendPointsData(Ps);
    }
}

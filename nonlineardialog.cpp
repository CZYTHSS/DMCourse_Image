#include "nonlineardialog.h"
#include "ui_nonlineardialog.h"

NonLinearDialog::NonLinearDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NonLinearDialog)
{
    ui->setupUi(this);
    connect(ui->LogApplyButton, SIGNAL(clicked(bool)), this, SLOT(ApplyLog()));
    connect(this, SIGNAL(sendLogParams(Params)), parent, SLOT(ReceiveParams(Params)));
    connect(ui->ExpApplyButton, SIGNAL(clicked(bool)), this, SLOT(ApplyExp()));
    connect(ui->GammaApplyButton, SIGNAL(clicked(bool)), this, SLOT(ApplyGamma()));
    connect(ui->RevokeButton, SIGNAL(clicked(bool)), this, SLOT(Revoke()));
}

NonLinearDialog::~NonLinearDialog()
{
    delete ui;
}

void NonLinearDialog::ApplyLog(){
    double a,b,c;
    a = ui->Log_a->value();
    b = ui->Log_b->value();
    c = ui->Log_c->value();
    if(b == 0 || c <= 0 || c == 1){
        QMessageBox::critical(0 ,
        "错误" , "b不可等于零，c必须比零更大且不为一!",
        QMessageBox::Ok | QMessageBox::Default ,
        QMessageBox::Cancel | QMessageBox::Escape , 	0 );
    }
    else{
        Params p;
        p.log_a = a;
        p.log_b = b;
        p.log_c = c;
        p.type = LOG;
        Coordinate co;
        co.CreateCoordinate(600, 600, 400, 400, 50, 50);
        Point last;
        for(int i = 0; i <= 255; i++){
            double x, y;
            x = i;
            y = a + log(x + 1)/(b * log(c));
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
        emit sendLogParams(p);
    }
}

void NonLinearDialog::ApplyExp(){
    double a,b,c;
    a = ui->Exp_a->value();
    b = ui->Exp_b->value();
    c = ui->Exp_c->value();
    if(b == 0 && c == 0){
        QMessageBox::critical(0 ,
        "错误" , "b and c can't both be zero!",
        QMessageBox::Ok | QMessageBox::Default ,
        QMessageBox::Cancel | QMessageBox::Escape , 	0 );
    }
    else{
        Params p;
        p.exp_a = a;
        p.exp_b = b;
        p.exp_c = c;
        p.type = EXP;
        Coordinate co;
        co.CreateCoordinate(600, 600, 400, 400, 50, 50);
        Point last;
        for(int i = 0; i <= 255; i++){
            double x, y;
            x = i;
            y = pow(b, c*(x - a)) - 1;
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
        emit sendLogParams(p);
    }
}

void NonLinearDialog::ApplyGamma(){
    double gamma,c;
    c = ui->Gamma_c->value();
    gamma = ui->Gamma_gamma->value();

    if(gamma < 0 || c < 0){
        QMessageBox::critical(0 ,
        "错误" , "b and gamma can't be smaller than zero!",
        QMessageBox::Ok | QMessageBox::Default ,
        QMessageBox::Cancel | QMessageBox::Escape , 	0 );
    }
    else{
        Params p;
        p.g_c = c;
        p.g_gamma = gamma;
        p.type = GAMMA;

        Coordinate co;
        co.CreateCoordinate(600, 600, 400, 400, 50, 50);
        Point last;
        for(int i = 0; i <= 255; i++){
            double x, y;
            x = i;
            y = c * pow(x, gamma);
            if(i == 0){
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

        emit sendLogParams(p);
    }
}

void NonLinearDialog::Revoke(){
    double gamma,c;
    c = 1;
    gamma = 1;

    if(gamma < 0 || c < 0){
        QMessageBox::critical(0 ,
        "错误" , "b and gamma can't be smaller than zero!",
        QMessageBox::Ok | QMessageBox::Default ,
        QMessageBox::Cancel | QMessageBox::Escape , 	0 );
    }
    else{
        Params p;
        p.g_c = c;
        p.g_gamma = gamma;
        p.type = GAMMA;
        emit sendLogParams(p);
    }
}

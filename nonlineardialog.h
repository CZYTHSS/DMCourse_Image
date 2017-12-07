#ifndef NONLINEARDIALOG_H
#define NONLINEARDIALOG_H

#include <QDialog>
#include <QDebug>
#include "lineardialog.h"
#include "utils.h"

enum NLTYPE:int{
    LOG = 1,
    EXP = 2,
    GAMMA = 3
};

class Params{
public:
    double log_a, log_b, log_c;
    double exp_a, exp_b, exp_c;
    double g_c, g_gamma;
    NLTYPE type;
};

namespace Ui {
class NonLinearDialog;
}

class NonLinearDialog : public QDialog
{
    Q_OBJECT

signals:
    void sendLogParams(Params p);
public slots:
    void ApplyLog();
    void ApplyExp();
    void ApplyGamma();
    void Revoke();
public:
    explicit NonLinearDialog(QWidget *parent = 0);
    ~NonLinearDialog();

private:
    Ui::NonLinearDialog *ui;
};

#endif // NONLINEARDIALOG_H

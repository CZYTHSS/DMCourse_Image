#ifndef LINEARDIALOG_H
#define LINEARDIALOG_H

#include <QDialog>
#include <QDebug>
#include <QMessageBox>
#include "utils.h"

class Points{
public:
    int x1;
    int x2;
    int y1;
    int y2;
};

namespace Ui {
class LinearDialog;
}

class LinearDialog : public QDialog
{
    Q_OBJECT

signals:
    void sendPointsData(Points Ps);

public slots:
    void ApplyLinearTrans();
    void Revoke();
public:
    explicit LinearDialog(QWidget *parent = 0);
    ~LinearDialog();

private:
    Ui::LinearDialog *ui;
};

#endif // LINEARDIALOG_H

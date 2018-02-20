#ifndef FORMLSTALGORITHMS_H
#define FORMLSTALGORITHMS_H

#include <QDialog>

namespace Ui {
class formLstAlgorithms;
}

class formLstAlgorithms : public QDialog
{
    Q_OBJECT

public:
    explicit formLstAlgorithms(QWidget *parent = 0);
    ~formLstAlgorithms();

private slots:
    void on_pbNDVI_clicked();

private:
    Ui::formLstAlgorithms *ui;
};

#endif // FORMLSTALGORITHMS_H

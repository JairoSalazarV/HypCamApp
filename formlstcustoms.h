#ifndef FORMLSTCUSTOMS_H
#define FORMLSTCUSTOMS_H

#include <QDialog>

namespace Ui {
class formLstCustoms;
}

class formLstCustoms : public QDialog
{
    Q_OBJECT

public:
    void funcShowMsgERROR_Timeout(QString msg, int ms=2000);

    void funcShowMsgSUCCESS_Timeout(QString msg);

    explicit formLstCustoms(QWidget *parent = 0);
    ~formLstCustoms();

private slots:
    void on_pbGetSlideSettings_clicked();

    void on_pbCreateFile_clicked();

private:
    Ui::formLstCustoms *ui;
};

#endif // FORMLSTCUSTOMS_H

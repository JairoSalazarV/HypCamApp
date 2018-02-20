#ifndef FORMLSTSETTINGS_H
#define FORMLSTSETTINGS_H

#include <QDialog>

namespace Ui {
class formLstSettings;
}

class formLstSettings : public QDialog
{
    Q_OBJECT

public:
    explicit formLstSettings(QWidget *parent = 0);
    ~formLstSettings();

private slots:

    void on_pbCameraSettings_clicked();

    void on_pbNDVI_clicked();

private:
    Ui::formLstSettings *ui;
};

#endif // FORMLSTSETTINGS_H

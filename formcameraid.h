#ifndef FORMCAMERAID_H
#define FORMCAMERAID_H

#include <QDialog>

namespace Ui {
class formCameraID;
}

class formCameraID : public QDialog
{
    Q_OBJECT

public:
    explicit formCameraID(QWidget *parent = 0);
    ~formCameraID();

private slots:
    void on_pushButton_clicked();

private:
    Ui::formCameraID *ui;
};

#endif // FORMCAMERAID_H

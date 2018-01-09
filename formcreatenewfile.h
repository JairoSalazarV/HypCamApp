#ifndef FORMCREATENEWFILE_H
#define FORMCREATENEWFILE_H

#include <QDialog>

namespace Ui {
class formCreateNewFile;
}

class formCreateNewFile : public QDialog
{
    Q_OBJECT

public:
    explicit formCreateNewFile(QWidget *parent = 0);
    ~formCreateNewFile();

private slots:
    void on_pbClose_clicked();

    void on_pbSave_clicked();

    void on_pbOpen_clicked();

private:
    Ui::formCreateNewFile *ui;
};

#endif // FORMCREATENEWFILE_H

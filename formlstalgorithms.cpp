#include "formlstalgorithms.h"
#include "ui_formlstalgorithms.h"
#include <formimageedition.h>

formLstAlgorithms::formLstAlgorithms(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::formLstAlgorithms)
{
    ui->setupUi(this);
}

formLstAlgorithms::~formLstAlgorithms()
{
    delete ui;
}

void formLstAlgorithms::on_pbNDVI_clicked()
{
    formImageEdition* tmpForm = new formImageEdition(this);
    tmpForm->setModal(true);
    tmpForm->show();
}

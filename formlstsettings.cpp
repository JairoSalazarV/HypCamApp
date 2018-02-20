#include "formlstsettings.h"
#include "ui_formlstsettings.h"
#include <formsettings.h>
#include <formndvisettings.h>
#include <__common.h>

formLstSettings::formLstSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::formLstSettings)
{
    ui->setupUi(this);
}

formLstSettings::~formLstSettings()
{
    delete ui;
}

void formLstSettings::on_pbCameraSettings_clicked()
{
    formSettings* tmpForm = new formSettings(this);
    tmpForm->setModal(true);
    tmpForm->show();
}

void formLstSettings::on_pbNDVI_clicked()
{
    formNDVISettings* tmpForm = new formNDVISettings(this);
    tmpForm->setModal(true);
    tmpForm->show();
}

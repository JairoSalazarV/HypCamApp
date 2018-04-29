#include "formcameraid.h"
#include "ui_formcameraid.h"
#include "__common.h"

formCameraID::formCameraID(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::formCameraID)
{
    ui->setupUi(this);

    QString IP = readAllFile(_FILENAME_CAMERA_IP).trimmed();
    if( IP.isEmpty() )
    {
        ui->txtIp->setText("192.168.1.68");
    }
    else
    {
        ui->txtIp->setText( IP );
    }
}

formCameraID::~formCameraID()
{
    delete ui;
}

void formCameraID::on_pushButton_clicked()
{
    if( saveFile( _FILENAME_CAMERA_IP, ui->txtIp->text().trimmed() ) == true )
    {
        funcShowMsg("Success","IP Saved");
    }
}

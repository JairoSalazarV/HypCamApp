#include "hypcamapp.h"
#include "ui_hypcamapp.h"

#include <__common.h>
#include <formlstsettings.h>
#include <formlstcustoms.h>
#include <formlstcamactions.h>
#include <formlstalgorithms.h>


HypCamApp::HypCamApp(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::HypCamApp)
{
    ui->setupUi(this);

    /*
    //Set Camera Default Parameters
    camSelected->isConnected = true;
    camSelected->On = true;
    camSelected->tcpPort = 51717;
    memset(camSelected->IP,'\0',15);
    memcpy(camSelected->IP,"172.24.1.1",strlen("172.24.1.1"));*/

    //Set FileDB
    funcSetFileDB();



}

HypCamApp::~HypCamApp()
{
    delete ui;
}

void HypCamApp::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);

    /*
    //Lock Screen Rotation
    QSize size;
    QRect rec = screenResolution(this);
    if( rec.width() > rec.height() )
    {
        size.setWidth(rec.height());
        size.setHeight(rec.width());
    }
    else
    {
        size.setHeight(rec.height());
        size.setWidth(rec.width());
    }
    this->setFixedSize(size);
    */

}

void HypCamApp::on_pbSettings_clicked()
{
    formLstSettings* tmpForm = new formLstSettings(this);
    tmpForm->setModal(true);
    tmpForm->show();
}

void HypCamApp::on_pbSynSettings_clicked()
{
    formLstCustoms* tmpForm = new formLstCustoms(this);
    tmpForm->setModal(true);
    tmpForm->show();
}

void HypCamApp::on_pbImgEdition_clicked()
{
    formLstCamActions* tmpForm = new formLstCamActions(this);
    tmpForm->setModal(true);
    tmpForm->show();
}

void HypCamApp::on_pbAnalysis_clicked()
{
    formLstAlgorithms* tmpForm = new formLstAlgorithms(this);
    tmpForm->setModal(true);
    tmpForm->show();
}

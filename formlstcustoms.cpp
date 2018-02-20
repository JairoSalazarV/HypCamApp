#include "formlstcustoms.h"
#include "ui_formlstcustoms.h"
#include <formcreatenewfile.h>
#include <__common.h>
#include <rasphypcam.h>
#include <QMessageBox>
#include <QTimer>

formLstCustoms::formLstCustoms(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::formLstCustoms)
{
    ui->setupUi(this);
}

formLstCustoms::~formLstCustoms()
{
    delete ui;
}

void formLstCustoms::on_pbGetSlideSettings_clicked()
{
    if( funcShowMsgYesNo("Synchronization","Please, confirm command") )
    {
        if( obtainFile( _PATH_REMOTE_SLIDESETTINGS, _PATH_LOCAL_SLIDESETTINGS, Q_NULLPTR ) )
            funcShowMsgSUCCESS_Timeout("Synchronization Successfully");
        else
            funcShowMsgERROR_Timeout("Sorry, Synchronization not Completed");
    }
}

void formLstCustoms::funcShowMsgERROR_Timeout(QString msg, int ms)
{
    QMessageBox *msgBox         = new QMessageBox(QMessageBox::Warning,"ERROR",msg,NULL);
    QTimer *msgBoxCloseTimer    = new QTimer(this);
    msgBoxCloseTimer->setInterval(ms);
    msgBoxCloseTimer->setSingleShot(true);
    connect(msgBoxCloseTimer, SIGNAL(timeout()), msgBox, SLOT(reject()));
    msgBoxCloseTimer->start();
    msgBox->exec();
}

void formLstCustoms::funcShowMsgSUCCESS_Timeout(QString msg)
{
    QMessageBox *msgBox         = new QMessageBox(QMessageBox::Warning,"SUCCESS",msg,NULL);
    QTimer *msgBoxCloseTimer    = new QTimer(this);
    msgBoxCloseTimer->setInterval(2500);
    msgBoxCloseTimer->setSingleShot(true);
    connect(msgBoxCloseTimer, SIGNAL(timeout()), msgBox, SLOT(reject()));
    msgBoxCloseTimer->start();
    msgBox->exec();
}

void formLstCustoms::on_pbCreateFile_clicked()
{
    formCreateNewFile* tmpForm = new formCreateNewFile(this);
    tmpForm->setModal(true);
    tmpForm->show();
}

#include "formimageedition.h"
#include "ui_formimageedition.h"

#include <__common.h>
#include <QMessageBox>
#include <QTimer>
#include <rasphypcam.h>
#include <unistd.h>
#include <QFileDialog>

formImageEdition::formImageEdition(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::formImageEdition)
{
    ui->setupUi(this);

    //Refresh Image to Show
    updateLastSnapshotAcquired();

    //Refresh Progress Bar
    ui->progBar->setVisible(false);
    ui->label->setText("");
}

formImageEdition::~formImageEdition()
{
    delete ui;
}

void formImageEdition::updateLastSnapshotAcquired()
{
    //Get Last Snapshot Filename
    if( fileExists(_FILENAME_LAST_SNAPSHOT) != 1 )
    {
        funcShowMsgERROR("Not Previous Snapshot Saved");
        ui->pbSynLastSnapshot->setEnabled(true);
        return (void)false;
    }

    //Load Image into RAM
    QString remoteSnapshot;
    remoteSnapshot = readFileParam(_PATH_LAST_USED_IMG_FILENAME);
    remoteSnapshot = remoteSnapshot.trimmed();
    //remoteSnapshot  = "./tmpSnapshots/" + readAllFile(_FILENAME_LAST_SNAPSHOT).trimmed() + "." + _FRAME_RECEIVED_EXTENSION;
    globalImgToShow = QImage(remoteSnapshot);

    //Update Image
    funcUpdateShownImage(globalImgToShow);

}

void formImageEdition::funcUpdateShownImage(QImage imgToShow)
{
    //Load image to display
    QPixmap pix = QPixmap::fromImage(imgToShow);
    int maxW, maxH;
    maxW = 492;
    maxH = 600;
    pix = pix.scaledToHeight(maxH);
    if( pix.width() > maxW )
        pix = pix.scaledToWidth(maxW);
    //It creates the scene to be loaded into Layout
    QGraphicsScene *sceneCalib = new QGraphicsScene(0,0,pix.width(),pix.height());
    ui->gvImgShown->setBackgroundBrush(QBrush(Qt::black));
    ui->gvImgShown->setBackgroundBrush(pix);
    ui->gvImgShown->setScene( sceneCalib );
    ui->gvImgShown->resize(pix.width(),pix.height());
    ui->gvImgShown->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    ui->gvImgShown->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    ui->gvImgShown->update();
}

void formImageEdition::on_pbSynLastSnapshot_clicked()
{
    ui->pbSynLastSnapshot->setEnabled(false);

    //Get Last Snapshot Filename
    if( fileExists(_FILENAME_LAST_SNAPSHOT) != 1 )
    {
        funcShowMsgERROR("Not Previous Snapshot Saved");
        ui->pbSynLastSnapshot->setEnabled(true);
        return (void)false;
    }

    //Prepare Progress Bar
    ui->progBar->setVisible(true);
    ui->progBar->setValue(0);
    ui->progBar->update();
    //QtDelay(300);

    QString remoteSnapshot, localSnapshot;
    remoteSnapshot  = "./tmpSnapshots/" + readAllFile(_FILENAME_LAST_SNAPSHOT).trimmed() + "." + _FRAME_RECEIVED_EXTENSION;
    localSnapshot   = remoteSnapshot;

    //Get Remote File    
    if( obtainFile( remoteSnapshot.toStdString(), localSnapshot.toStdString(), ui->progBar ) == _OK )
    {
        //Get Image Into RAM
        globalImgToShow = QImage(localSnapshot);
        funcUpdateShownImage(globalImgToShow);
        ui->pbSynLastSnapshot->setEnabled(true);
        ui->label->setText("");
        QtDelay(20);

        //Save last file open
        saveFile(_PATH_LAST_USED_IMG_FILENAME,localSnapshot);

        //Finish
        funcShowMsgSUCCESS_Timeout("Image Acquired Successfully");
    }
    else
    {
        funcShowMsgERROR_Timeout("Sorry, Synchronization not Completed");
    }
}


void formImageEdition::funcShowMsgSUCCESS_Timeout(QString msg, int ms)
{
    QMessageBox *msgBox         = new QMessageBox(QMessageBox::Warning,"SUCCESS",msg,NULL);
    QTimer *msgBoxCloseTimer    = new QTimer(this);
    msgBoxCloseTimer->setInterval(ms);
    msgBoxCloseTimer->setSingleShot(true);
    connect(msgBoxCloseTimer, SIGNAL(timeout()), msgBox, SLOT(reject()));
    msgBoxCloseTimer->start();
    msgBox->exec();
}

void formImageEdition::funcShowMsgSUCCESS_Timeout(QString msg)
{
    QMessageBox *msgBox         = new QMessageBox(QMessageBox::Warning,"SUCCESS",msg,NULL);
    QTimer *msgBoxCloseTimer    = new QTimer(this);
    msgBoxCloseTimer->setInterval(2500);
    msgBoxCloseTimer->setSingleShot(true);
    connect(msgBoxCloseTimer, SIGNAL(timeout()), msgBox, SLOT(reject()));
    msgBoxCloseTimer->start();
    msgBox->exec();
}


void formImageEdition::funcShowMsgERROR_Timeout(QString msg, int ms)
{
    QMessageBox *msgBox         = new QMessageBox(QMessageBox::Warning,"ERROR",msg,NULL);
    QTimer *msgBoxCloseTimer    = new QTimer(this);
    msgBoxCloseTimer->setInterval(ms);
    msgBoxCloseTimer->setSingleShot(true);
    connect(msgBoxCloseTimer, SIGNAL(timeout()), msgBox, SLOT(reject()));
    msgBoxCloseTimer->start();
    msgBox->exec();
}

void formImageEdition::funcShowMsgERROR_Timeout(QString msg)
{
    QMessageBox *msgBox         = new QMessageBox(QMessageBox::Warning,"ERROR",msg,NULL);
    QTimer *msgBoxCloseTimer    = new QTimer(this);
    msgBoxCloseTimer->setInterval(2500);
    msgBoxCloseTimer->setSingleShot(true);
    connect(msgBoxCloseTimer, SIGNAL(timeout()), msgBox, SLOT(reject()));
    msgBoxCloseTimer->start();
    msgBox->exec();
}

void formImageEdition::on_pbNDVIAlgorithm_clicked()
{
    QString stringThreshold;
    QString stringBrilliant;
    stringThreshold = readFileParam(_PATH_NDVI_THRESHOLD);
    stringBrilliant = readFileParam(_PATH_NDVI_BRILLIANT);
    int makeBrilliant = (stringBrilliant.toInt(0)==1)?1:0;

    ui->label->setText("Calculating NDVI...");
    QtDelay(20);
    funcNDVI( &globalImgToShow, stringThreshold.toDouble(0), makeBrilliant, ui->progBar );
    funcUpdateShownImage(globalImgToShow);
    ui->label->setText("");
    QtDelay(20);
}

void formImageEdition::on_pbSaveImage_clicked()
{
    //
    //Read the filename
    //
    QString fileName;
    QString lastPath = readFileParam(_PATH_LAST_IMG_SAVED);
    if( lastPath.isEmpty() )//First time using this parameter
    {
        lastPath = "./snapshots/";
    }
    fileName = QFileDialog::getSaveFileName(
                                                this,
                                                tr("Save Snapshot as..."),
                                                lastPath,
                                                tr("Documents (*.png)")
                                            );
    if( fileName.isEmpty() )
    {
        qDebug() << "Filename not typed";
        return (void)false;
    }
    else
    {
        lastPath = funcRemoveFileNameFromPath(fileName);
        saveFile(_PATH_LAST_IMG_SAVED,lastPath);
    }

    //
    //Validate filename
    //
    fileName = funcRemoveImageExtension(fileName);

    //
    //Save last file open
    //
    saveFile(_PATH_LAST_USED_IMG_FILENAME,fileName);

    //
    //Save image
    //
    ui->label->setText("Saving Image Locally...");
    globalImgToShow.save(fileName);
    ui->label->setText(" ");

}

void formImageEdition::on_pbOpenImage_clicked()
{
    QString lastPath = readFileParam(_PATH_LAST_IMG_OPEN);
    if( lastPath.isEmpty() )//First time using this parameter
    {
        lastPath = "./snapshots/Calib/";
    }

    //Select image
    //..
    QString auxQstring;
    auxQstring = QFileDialog::getOpenFileName(
                                                this,
                                                tr("Select image..."),
                                                lastPath,
                                                "(*.ppm *.RGB888 *.tif *.png *.jpg *.jpeg *.JPEG *.JPG *.bmp);;"
                                             );
    if( auxQstring.isEmpty() )
    {
        return (void)NULL;
    }
    else
    {
        //Save last file open
        saveFile(_PATH_LAST_USED_IMG_FILENAME,auxQstring);

        //Save Folder in order to Speed up File Selection
        lastPath = funcRemoveFileNameFromPath(auxQstring);
        saveFile(_PATH_LAST_IMG_OPEN,lastPath);
    }

    //Load Image Selected
    globalImgToShow = QImage(auxQstring);
    funcUpdateShownImage(globalImgToShow);
}

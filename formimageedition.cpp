#include "formimageedition.h"
#include "ui_formimageedition.h"

#include <__common.h>
#include <QMessageBox>
#include <QTimer>
#include <rasphypcam.h>
#include <unistd.h>

formImageEdition::formImageEdition(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::formImageEdition)
{
    ui->setupUi(this);

    //Refresh Image to Show
    funcUpdateShownImage();

    ui->progBar->setVisible(false);
    ui->label->setText("");
}

formImageEdition::~formImageEdition()
{
    delete ui;
}

void formImageEdition::funcUpdateShownImage()
{
    if( fileExists(_PATH_DISPLAY_IMAGE) )
    {

        //Load image to display
        QPixmap pix( _PATH_DISPLAY_IMAGE );
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
        ui->label->setText("Saving Image Locally...");
        QImage receivedImg(localSnapshot);
        funcUpdateShownImage(receivedImg);
        ui->pbSynLastSnapshot->setEnabled(true);
        QtDelay(20);
        receivedImg.save(_PATH_DISPLAY_IMAGE);
        ui->label->setText("");
        QtDelay(20);
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

/*
int formImageEdition::obtainFile(std::string remoteFile, std::string localFile )
{
    if( funcRaspFileExists( remoteFile ) == 0 )
    {
        debugMsg("File does not exists");
        return _ERROR;
    }
    else
    {
        int fileLen = 0;
        u_int8_t* fileReceived = funcQtReceiveFile( remoteFile, &fileLen );

        //funcShowMsg_Timeout("",QString::fromStdString(remoteFile));

        //funcShowMsg_Timeout("fileLen",QString::number(fileLen));

        if( saveBinFile_From_u_int8_T(localFile,fileReceived,fileLen) )
            return _OK;
        else
            return _ERROR;
    }
    return _OK;
}
*/

/*
u_int8_t* formImageEdition::funcQtReceiveFile( std::string fileNameRequested, int* fileLen )
{

    //It assumes that file exists and this was verified by command 10
    //
    //Receive the path of a file into raspCamera and
    //return the file if exists or empty arry otherwise
    *fileLen = 0;

    structCamSelected imgEdiCamSel;

    //Connect to socket
    int socketID = connectSocket( &imgEdiCamSel );
    if( socketID == -1 )
    {
        //funcShowMsgERROR_Timeout("ERROR connecting to socket");
        return (u_int8_t*)NULL;
    }

    //Request file
    int n;
    strReqFileInfo *reqFileInfo = (strReqFileInfo*)malloc(sizeof(strReqFileInfo));
    memset( reqFileInfo, '\0', sizeof(strReqFileInfo) );
    reqFileInfo->idMsg          = 11;
    reqFileInfo->fileNameLen    = fileNameRequested.size();
    memcpy( &reqFileInfo->fileName[0], fileNameRequested.c_str(), fileNameRequested.size() );
    int tmpFrameLen = sizeof(strReqFileInfo);//IdMsg + lenInfo + fileLen + padding
    n = ::write( socketID, reqFileInfo, tmpFrameLen+1 );
    if (n < 0){
        //funcShowMsgERROR_Timeout("ERROR writing to socket");
        return (u_int8_t*)NULL;
    }

    //Receive file's size
    *fileLen = funcReceiveOnePositiveInteger( socketID );
    if( *fileLen < 1 )
    {
        //funcShowMsgERROR_Timeout("*fileLen = " + QString::number(*fileLen));
    }

    //Prepare container
    u_int8_t* theFILE = (u_int8_t*)malloc(*fileLen);
    bzero( &theFILE[0], *fileLen );

    //Read file from system buffer
    int filePos     = 0;
    int remainder   = *fileLen;

    //Read image from buffer
    while( remainder > 0 )
    {
        n = read(socketID,(void*)&theFILE[filePos],remainder);
        remainder   -= n;
        filePos     += n;
    }

    //Close connection
    ::close(socketID);

    //Return file
    return theFILE;


}*/

void formImageEdition::on_pbNDVIAlgorithm_clicked()
{
    QImage* tmpImg = new QImage(_PATH_DISPLAY_IMAGE);
    QString stringThreshold;
    QString stringBrilliant;
    stringThreshold = readFileParam(_PATH_NDVI_THRESHOLD);
    stringBrilliant = readFileParam(_PATH_NDVI_BRILLIANT);
    int makeBrilliant = (stringBrilliant.toInt(0)==1)?1:0;

    ui->label->setText("Calculating NDVI...");
    QtDelay(500);
    funcNDVI( tmpImg, stringThreshold.toDouble(0), makeBrilliant, ui->progBar );
    funcUpdateShownImage(*tmpImg);
    ui->label->setText("");
    QtDelay(00);
}

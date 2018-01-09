#include "hypcamapp.h"
#include "ui_hypcamapp.h"

#include <QMessageBox>
#include <__common.h>
#include <hypCamAPI.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <formsettings.h>
#include <QTimer>
#include <rasphypcam.h>
#include <formtimertxt.h>
#include <formcreatenewfile.h>

//structSettings *lstSettings = (structSettings*)malloc(sizeof(structSettings));
structRaspcamSettings *mainRaspcamSettings = (structRaspcamSettings*)malloc(sizeof(structRaspcamSettings));
structCamSelected *camSelected = (structCamSelected*)malloc(sizeof(structCamSelected));

HypCamApp::HypCamApp(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::HypCamApp)
{
    ui->setupUi(this);

    //Set Camera Default Parameters
    camSelected->isConnected = true;
    camSelected->On = true;
    camSelected->tcpPort = 51717;
    memset(camSelected->IP,'\0',15);
    memcpy(camSelected->IP,"172.24.1.1",strlen("172.24.1.1"));

    //Set FileDB
    funcSetFileDB();



}

HypCamApp::~HypCamApp()
{
    delete ui;
}

void HypCamApp::on_pbCamConn_clicked()
{
    //Check if Camera Settings Exists
    //Initialize camera parameters
    if ( !fileExists( _PATH_RASPICAM_SETTINGS ) )
    {
        funcShowMsgERROR("Please, Set and Save Settings Before to Take a Picture.");
        return (void)false;
    }

    //Validating IP
    QString IP((const char*)camSelected->IP);
    if( !funcIsIP( IP.toStdString() ) ){
        funcShowMsg("ERROR","Invalid IP address");
    }else{
        //Test IP
        camSettings * tmpCamSett = (camSettings*)malloc(sizeof(camSettings));
        tmpCamSett->idMsg = (char)0;
        funcValCam(
                        IP.toStdString(),
                        camSelected->tcpPort,
                        tmpCamSett
                  );
        if( tmpCamSett->idMsg < 1 ){
            funcShowMsg("ERROR","Camera does not respond at "+IP);
        }else{
            funcShowMsg("SUCCESS","Camera Connected Successfully");
        }
    }
}

void HypCamApp::on_pbShutdown_clicked()
{
    if( funcShowMsgYesNo("ALERT","Turn off the Camera?") == 1 )
    {
        std::string cmdShutdown = "sudo shutdown -h now";
        bool ok;

        funcRemoteTerminalCommand(cmdShutdown,camSelected,0,false,&ok);
        if(ok == true)
        {
            funcShowMsg("SUCCESS","Camera Off");
        }
        else
        {
            funcShowMsgERROR("Command Not Applied");
        }
    }
}

std::string HypCamApp::funcRemoteTerminalCommand(
                                                    std::string command,
                                                    structCamSelected *camSelected,
                                                    int trigeredTime,
                                                    bool waitForAnswer,
                                                    bool* ok
){
    //* It is used when waitForAnswer==true
    //* Wait for answer when you need to know a parameter or the
    //  command result
    *ok = true;
    std::string tmpTxt;

    //--------------------------------------
    //Open socket
    //--------------------------------------
    int n;
    int sockfd = connectSocket( camSelected );
    qDebug() << "Socket opened";

    //--------------------------------------
    //Excecute command
    //--------------------------------------
    frameStruct *frame2send = (frameStruct*)malloc(sizeof(frameStruct));
    memset(frame2send,'\0',sizeof(frameStruct));
    frame2send->header.idMsg        = (waitForAnswer==true)?(unsigned char)2:(unsigned char)5;
    frame2send->header.trigeredTime = trigeredTime;
    frame2send->header.numTotMsg    = 1;
    frame2send->header.consecutive  = 1;
    frame2send->header.bodyLen      = command.length();
    bzero(frame2send->msg,command.length()+1);
    memcpy( frame2send->msg, command.c_str(), command.length() );

    //Request command result
    n = ::write(sockfd,frame2send,sizeof(frameStruct)+1);
    if(n<0){
        qDebug() << "ERROR: Excecuting Remote Command";
        *ok = false;
        return "";
    }

    //Receibing ack with file len
    unsigned char bufferRead[frameBodyLen];
    n = read(sockfd,bufferRead,frameBodyLen);

    if( waitForAnswer == true )
    {
        unsigned int fileLen;
        memcpy(&fileLen,&bufferRead,sizeof(unsigned int));
        fileLen = (fileLen<frameBodyLen)?frameBodyLen:fileLen;
        qDebug() << "fileLen: " << fileLen;
        //funcShowMsg("FileLen n("+QString::number(n)+")",QString::number(fileLen));

        //Receive File
        unsigned char tmpFile[fileLen];
        if( funcReceiveFile( sockfd, fileLen, bufferRead, tmpFile ) == false )
        {
            *ok = false;
        }
        tmpTxt.clear();
        tmpTxt.assign((char*)tmpFile);
        qDebug() <<tmpFile;
    }
    ::close(sockfd);

    //Return Command Result
    return tmpTxt;
}



bool HypCamApp::funcReceiveFile(
                                    int sockfd,
                                    unsigned int fileLen,
                                    unsigned char *bufferRead,
                                    unsigned char *tmpFile
){

    qDebug() << "Inside funcReceiveFile sockfd: " << sockfd;



    //Requesting file
    int i, n;

    n = ::write(sockfd,"sendfile",8);
    if (n < 0){
        qDebug() << "ERROR: writing to socket";
        return false;
    }



    //Receive file parts
    unsigned int numMsgs = (fileLen>0)?floor( (float)fileLen / (float)frameBodyLen ):0;
    numMsgs = ((numMsgs*frameBodyLen)<fileLen)?numMsgs+1:numMsgs;
    unsigned int tmpPos = 0;
    memset(tmpFile,'\0',fileLen);
    qDebug() << "Receibing... " <<  QString::number(numMsgs) << " messages";
    qDebug() << "fileLen: " << fileLen;

    //Receive the last
    if(numMsgs==0){
        //Receives the unik message
        bzero(bufferRead,frameBodyLen);
        //qDebug() << "R1";
        //n = read(sockfd,tmpFile,fileLen);
    }else{

        for(i=1;i<=(int)numMsgs;i++){
            bzero(bufferRead,frameBodyLen);
            n = read(sockfd,bufferRead,frameBodyLen);
            //qDebug() << "n: " << n;
            if(n!=(int)frameBodyLen&&i<(int)numMsgs){
                qDebug() << "ERROR, message " << i << "WRONG";
                return false;
            }
            //Append message to file
            memcpy( &tmpFile[tmpPos], bufferRead, frameBodyLen );
            tmpPos += n;
            //Request other part
            if( i<(int)numMsgs ){
                //qDebug() << "W2";
                QtDelay(2);
                n = ::write(sockfd,"sendpart",8);
                if(n<0){
                    qDebug() << "ERROR: Sending part-file request";
                    return false;
                }
            }
        }
    }

    return true;
}

void HypCamApp::funcShowMsg_Timeout(QString title, QString msg, int ms)
{
    QMessageBox *msgBox         = new QMessageBox(QMessageBox::Warning,title,msg,NULL);
    QTimer *msgBoxCloseTimer    = new QTimer(this);
    msgBoxCloseTimer->setInterval(ms);
    msgBoxCloseTimer->setSingleShot(true);
    connect(msgBoxCloseTimer, SIGNAL(timeout()), msgBox, SLOT(reject()));
    msgBoxCloseTimer->start();
    msgBox->exec();
}

void HypCamApp::funcShowMsg_Timeout(QString title, QString msg)
{
    QMessageBox *msgBox         = new QMessageBox(QMessageBox::Warning,title,msg,NULL);
    QTimer *msgBoxCloseTimer    = new QTimer(this);
    msgBoxCloseTimer->setInterval(2500);
    msgBoxCloseTimer->setSingleShot(true);
    connect(msgBoxCloseTimer, SIGNAL(timeout()), msgBox, SLOT(reject()));
    msgBoxCloseTimer->start();
    msgBox->exec();
}

void HypCamApp::funcShowMsgSUCCESS_Timeout(QString msg, int ms)
{
    QMessageBox *msgBox         = new QMessageBox(QMessageBox::Warning,"SUCCESS",msg,NULL);
    QTimer *msgBoxCloseTimer    = new QTimer(this);
    msgBoxCloseTimer->setInterval(ms);
    msgBoxCloseTimer->setSingleShot(true);
    connect(msgBoxCloseTimer, SIGNAL(timeout()), msgBox, SLOT(reject()));
    msgBoxCloseTimer->start();
    msgBox->exec();
}

void HypCamApp::funcShowMsgSUCCESS_Timeout(QString msg)
{
    QMessageBox *msgBox         = new QMessageBox(QMessageBox::Warning,"SUCCESS",msg,NULL);
    QTimer *msgBoxCloseTimer    = new QTimer(this);
    msgBoxCloseTimer->setInterval(2500);
    msgBoxCloseTimer->setSingleShot(true);
    connect(msgBoxCloseTimer, SIGNAL(timeout()), msgBox, SLOT(reject()));
    msgBoxCloseTimer->start();
    msgBox->exec();
}

void HypCamApp::funcShowMsgERROR_Timeout(QString msg, int ms)
{
    QMessageBox *msgBox         = new QMessageBox(QMessageBox::Warning,"ERROR",msg,NULL);
    QTimer *msgBoxCloseTimer    = new QTimer(this);
    msgBoxCloseTimer->setInterval(ms);
    msgBoxCloseTimer->setSingleShot(true);
    connect(msgBoxCloseTimer, SIGNAL(timeout()), msgBox, SLOT(reject()));
    msgBoxCloseTimer->start();
    msgBox->exec();
}

void HypCamApp::funcShowMsgERROR_Timeout(QString msg)
{
    QMessageBox *msgBox         = new QMessageBox(QMessageBox::Warning,"ERROR",msg,NULL);
    QTimer *msgBoxCloseTimer    = new QTimer(this);
    msgBoxCloseTimer->setInterval(2500);
    msgBoxCloseTimer->setSingleShot(true);
    connect(msgBoxCloseTimer, SIGNAL(timeout()), msgBox, SLOT(reject()));
    msgBoxCloseTimer->start();
    msgBox->exec();
}

void HypCamApp::on_pbSettings_clicked()
{
    formSettings* tmpForm = new formSettings(this);
    tmpForm->setModal(true);
    tmpForm->show();
}


void HypCamApp::on_pbVideo_clicked()
{
    QString remoteVideoID;
    funcMainCall_RecordVideo(&remoteVideoID);
}

void HypCamApp::funcMainCall_RecordVideo(QString* videoID, bool defaultPath, bool ROI)
{
    bool commandExecuted;

    videoID->clear();

    //---------------------------------------------------
    //Get Last Camera Settings
    //---------------------------------------------------
    if( funcGetRaspCamParameters() != _OK )
        return (void)false;

    //---------------------------------------------------
    //Get Video-ID Destine
    //---------------------------------------------------
    if( defaultPath )
    {
        videoID->append(_PATH_VIDEO_REMOTE_H264);
    }
    else
    {
        this->setVisible(false);
        *videoID = funcGetParam("Video-ID");
        if(videoID->isEmpty())
        {
            funcShowMsgERROR_Timeout("Invalid Video-ID");
            this->setVisible(true);
            return (void)false;
        }

        //---------------------------------------------------
        //Validate File/Dir Name
        //---------------------------------------------------
        QString remoteFile, localFile;
        remoteFile.append(_PATH_REMOTE_FOLDER_VIDEOS);
        remoteFile.append(*videoID);
        remoteFile.append(_VIDEO_EXTENSION);

        //Local File
        QString syncLocalFolder = funcGetSyncFolder();
        localFile.clear();
        localFile.append(syncLocalFolder);
        localFile.append(_PATH_REMOTE_FOLDER_VIDEOS);
        localFile.append(videoID);
        localFile.append(_VIDEO_EXTENSION);

        //Check if exists
        int itExists = funcValidateFileDirNameDuplicated( remoteFile, localFile );
        if( itExists != _OK )
        {
            if( itExists == -1 )
                funcShowMsgERROR_Timeout("Video-ID Exists Locally: Please, use another");
            else
                funcShowMsgERROR_Timeout("Video-ID Exists Remotelly: Please, use another");
            this->setVisible(true);
            return (void)false;
        }

        //---------------------------------------------------
        //Prepare Remote Scenary
        //---------------------------------------------------

        //Delete Remote File if Exists
        *videoID = _PATH_REMOTE_FOLDER_VIDEOS + *videoID + _VIDEO_EXTENSION;
        QString tmpCommand;
        tmpCommand.clear();
        tmpCommand.append("sudo rm "+ *videoID);
        funcRemoteTerminalCommand(tmpCommand.toStdString(),camSelected,0,false,&commandExecuted);
        if( !commandExecuted )
        {
            funcShowMsgERROR_Timeout("Deleting Remote videoID");
            return (void)false;
        }
    }

    //-----------------------------------------------------
    // Save snapshots settings
    //-----------------------------------------------------
    // Get camera resolution
    camRes = getCamRes( mainRaspcamSettings->CameraMp );

    //-----------------------------------------------------
    //Start to Record Remote Video
    //-----------------------------------------------------

    // Generate Video Command
    QString getRemVidCommand = genRemoteVideoCommand(mainRaspcamSettings,*videoID,ROI);

    //funcShowMsgSUCCESS_Timeout(getRemVidCommand,4000);


    // Execute Remote Command
    funcRemoteTerminalCommand(
                                getRemVidCommand.toStdString(),
                                camSelected,
                                mainRaspcamSettings->TriggeringTimeSecs,
                                false,
                                &commandExecuted
                            );
    if( !commandExecuted )
    {
        funcShowMsgERROR_Timeout("Starting Remote Recording");
        return (void)false;
    }

    //-----------------------------------------------------
    //Display Timer
    //-----------------------------------------------------
    //Before to Start Recording
    funcDisplayTimer("Countdown to Recording...",mainRaspcamSettings->TriggeringTimeSecs,Qt::black);

    //During Recording
    funcDisplayTimer("Recording...",mainRaspcamSettings->VideoDurationSecs,Qt::red);

    //Return
    this->setVisible(true);

}

int HypCamApp::funcGetRaspCamParameters()
{
    //---------------------------------------------------
    //Get Last Camera Settings
    //---------------------------------------------------
    //Reset Settings
    memset(mainRaspcamSettings,'\0',sizeof(structRaspcamSettings));

    //Get Setting Filename
    QString tmpLastCamSettings;
    if( !readFileParam( _PATH_LAST_CAM_SETTINGS, &tmpLastCamSettings) )
    {
        saveFile(_PATH_LAST_CAM_SETTINGS,_PATH_RASPICAM_SETTINGS);
        tmpLastCamSettings.clear();
        tmpLastCamSettings.append(_PATH_RASPICAM_SETTINGS);
    }

    //Initialize camera parameters
    if ( fileExists( tmpLastCamSettings ) )
    {
        funcGetRaspParamFromXML( mainRaspcamSettings, tmpLastCamSettings );
    }
    else
    {
        funcShowMsg_Timeout("ALERT: It is your first time...","Please, It is necessary to set and save settings.");
        return _ERROR;
    }

    return _OK;
}


int HypCamApp::funcValidateFileDirNameDuplicated(QString remoteFile, QString localFile)
{
    qDebug() << "remoteFile: " << remoteFile;
    qDebug() << "localFile: " << localFile;

    //Check if exists locally
    if( fileExists(localFile) )
    {
        qDebug() << "Exists Locally";
        return -1;
    }

    //Check if exists remotelly
    if( funcRaspFileExists( remoteFile.toStdString() ) == 1 )
    {
        qDebug() << "Exists Remotelly";
        return -2;
    }

    //File does not exists locally or remotelly
    return _OK;
}

QString HypCamApp::funcGetSyncFolder()
{
    QString tmpParam;
    if( !readFileParam( _PATH_LAST_SYNC_FOLDER, &tmpParam) )
    {
        saveFile(_PATH_LAST_SYNC_FOLDER,_PATH_LOCAL_SYNC_FOLDERS);
        tmpParam.clear();
        tmpParam.append(_PATH_LOCAL_SYNC_FOLDERS);
    }
    return tmpParam;
}


cameraResolution* HypCamApp::getCamRes( int camResSetting = 1 )
{
    //cameraResolution* camRes;
    //camRes = (cameraResolution*)malloc(sizeof(cameraResolution));

    if( camResSetting == 5 )
    {
        //#define _BIG_WIDTH 2592 //2592 | 640 | 320
        //#define _BIG_HEIGHT 1944 //1944 | 480 | 240
        camRes->width   = _RASP_CAM_5MP_IMAGE_W;
        camRes->height  = _RASP_CAM_5MP_IMAGE_H;
        camRes->videoW  = _RASP_CAM_5MP_VIDEO_W; //1920 | 1640
        camRes->videoH  = _RASP_CAM_5MP_VIDEO_H; //1080 | 1232
    }

    if( camResSetting == 8 )
    {
        //https://www.raspberrypi.org/forums/viewtopic.php?t=145815
        camRes->width   = _RASP_CAM_8MP_IMAGE_W;
        camRes->height  = _RASP_CAM_8MP_IMAGE_H;
        camRes->videoW  = _RASP_CAM_8MP_VIDEO_W; //1920 | 1640
        camRes->videoH  = _RASP_CAM_8MP_VIDEO_H; //1080 | 1232
    }

    return camRes;

}

QString HypCamApp::genRemoteVideoCommand(structRaspcamSettings* raspcamSettings, QString remoteVideo, bool ROI)
{
    QString tmpCommand;
    tmpCommand.append("raspivid -n -t ");
    tmpCommand.append( QString::number( raspcamSettings->VideoDurationSecs*1000 ) );
    tmpCommand.append( " -vf -b 50000000 -fps " ); // -b -> bitrate
    tmpCommand.append( QString::number(_VIDEO_FRAME_RATE) );
    tmpCommand.append( " -o " );
    tmpCommand.append( remoteVideo );

    //.................................
    //Diffraction Area ROI
    //.................................
    if( ROI == true )
    {
        double W, H;
        squareAperture *aperture = (squareAperture*)malloc(sizeof(squareAperture));
        memset(aperture,'\0',sizeof(squareAperture));
        if( !funGetSquareXML( _PATH_SLIDE_DIFFRACTION, aperture ) )
        {
            funcShowMsg("ERROR","Loading Usable Area in Pixels: _PATH_SLIDE_DIFFRACTION");
            tmpCommand.clear();
            return tmpCommand;
        }
        W = (double)aperture->rectW/(double)aperture->canvasW;
        H = (double)aperture->rectH/(double)aperture->canvasH;
        qDebug() << "W: " << W << " H: " << H;

        tmpCommand.append(" -roi ");
        tmpCommand.append(QString::number((double)aperture->rectX/(double)aperture->canvasW)+",");
        tmpCommand.append(QString::number((double)aperture->rectY/(double)aperture->canvasH)+",");
        tmpCommand.append(QString::number(W)+",");
        tmpCommand.append(QString::number(H));

        //.................................
        //Image Size
        //.................................
        camRes          = getCamRes( raspcamSettings->CameraMp );
        //Width
        tmpCommand.append(" -w ");
        tmpCommand.append(QString::number( aperture->rectW ));
        //Height
        tmpCommand.append(" -h ");
        tmpCommand.append(QString::number( aperture->rectH ));
    }
    else
    {
        //.................................
        //Image Size: Full Resolution
        //.................................
        camRes          = getCamRes( raspcamSettings->CameraMp );
        //Width
        tmpCommand.append(" -w ");
        tmpCommand.append(QString::number( camRes->videoW ));
        //Height
        tmpCommand.append(" -h ");
        tmpCommand.append(QString::number( camRes->videoH ));
    }



    //.................................
    //Colour balance?
    //.................................
    if( raspcamSettings->ColorBalance ){
        tmpCommand.append(" -ifx colourbalance");
    }

    //.................................
    //Denoise?
    //.................................
    if( raspcamSettings->Denoise ){
        tmpCommand.append(" -ifx denoise");
    }

    //.................................
    //Diffraction Shuter speed
    //.................................
    int shutSpeed = raspcamSettings->ShutterSpeedMs;
    if( shutSpeed > 0 ){
        tmpCommand.append(" -ss " + QString::number(shutSpeed));
    }

    //.................................
    //AWB
    //.................................
    std::string sAWB((char*)raspcamSettings->AWB, sizeof(raspcamSettings->AWB));
    if( strcmp(sAWB.c_str(),"none") != 0 ){
        tmpCommand.append(" -awb ");
        tmpCommand.append(sAWB.c_str());
    }

    //.................................
    //Exposure
    //.................................
    std::string sExposure((char*)raspcamSettings->Exposure, sizeof(raspcamSettings->Exposure));
    if( strcmp(sExposure.c_str(),"none") != 0 ){
        tmpCommand.append(" -ex ");
        tmpCommand.append(sExposure.c_str());
    }

    //.................................
    //ISO
    //.................................
    if( raspcamSettings->ISO > 0 ){
        tmpCommand.append(" -ISO " + QString::number(raspcamSettings->ISO) );
    }

    //.................................
    //Flipped
    //.................................
    if( raspcamSettings->Flipped ){
        tmpCommand.append(" -vf " );
    }


    return tmpCommand;
}

int HypCamApp::funcDisplayTimer(QString title, int timeMs, QColor color)
{
    if( timeMs > 0 )
    {
        formTimerTxt* timerTxt = new formTimerTxt(this,title,timeMs,color);
        timerTxt->setModal(true);
        timerTxt->show();
        QtDelay(200);
        timerTxt->startMyTimer(timeMs);
    }
    return 1;
}

void HypCamApp::on_pbCreateFile_clicked()
{
    formCreateNewFile* tmpForm = new formCreateNewFile(this);
    tmpForm->setModal(true);
    tmpForm->show();
}

void HypCamApp::on_pbSnapshot_clicked()
{
    funcMainCall_GetSnapshot();
}

void HypCamApp::funcMainCall_GetSnapshot()
{

    this->setVisible(false);

    //---------------------------------------------------
    //Get Last Camera Settings
    //---------------------------------------------------
    if( funcGetRaspCamParameters() != _OK )
    {
        this->setVisible(true);
        return (void)false;
    }

    //---------------------------------------------------
    //Get Snapshot-ID Destine
    //---------------------------------------------------
    QString fileName;
    fileName = funcGetParam("Snapshot-ID");
    if(fileName.isEmpty())
    {
        funcShowMsgERROR_Timeout("Invalid Snapshot-ID");
        this->setVisible(true);
        return (void)false;
    }

    //---------------------------------------------------
    //Validate File/Dir Name
    //---------------------------------------------------
    QString remoteFile, localFile;
    remoteFile = _PATH_REMOTE_FOLDER_SNAPSHOTS + fileName + _SNAPSHOT_REMOTE_EXTENSION;

    //Local File
    QString syncFolder;
    syncFolder = funcGetSyncFolder();
    localFile.clear();
    localFile.append(syncFolder);
    localFile.append(_PATH_LOCAL_FOLDER_SNAPSHOTS);
    localFile.append(fileName);
    localFile.append(_SNAPSHOT_REMOTE_EXTENSION);

    if( !funcValidateFileDirNameDuplicated( remoteFile, localFile ) )
    {
        funcShowMsgERROR_Timeout("Snapshot-ID Exists: Please, use another");
        this->setVisible(true);
        return (void)false;
    }

    //--------------------------------------
    //Prepare Remote Scenary
    //--------------------------------------

    //Delete Remote File if Exists
    fileName = _PATH_REMOTE_FOLDER_SNAPSHOTS + fileName + _SNAPSHOT_REMOTE_EXTENSION;
    QString tmpCommand;
    tmpCommand.clear();
    tmpCommand.append("sudo rm "+ fileName);
    bool commandExecuted;
    funcRemoteTerminalCommand(tmpCommand.toStdString(),camSelected,0,false,&commandExecuted);
    if( !commandExecuted )
    {
        funcShowMsgERROR_Timeout("Deleting Remote Snapshot-ID");
        this->setVisible(true);
        return (void)false;
    }

    //-----------------------------------------------------
    // Save snapshots settings
    //-----------------------------------------------------

    // Get camera resolution
    camRes = getCamRes( mainRaspcamSettings->CameraMp );

    //-----------------------------------------------------
    // Take Remote Snapshot
    //-----------------------------------------------------
    if( !takeRemoteSnapshot(fileName,false) )
    {
        funcShowMsgERROR_Timeout("Taking Full Area Snapshot");
        this->setVisible(true);
        return (void)false;
    }

    //-----------------------------------------------------
    //Start Timer
    //-----------------------------------------------------
    int triggeringTime;
    triggeringTime = mainRaspcamSettings->TriggeringTimeSecs;
    if( triggeringTime > 0 )
    {
        formTimerTxt* timerTxt = new formTimerTxt(this,"Remainning Time to Shoot...",triggeringTime);
        timerTxt->setModal(true);
        timerTxt->show();
        QtDelay(200);
        timerTxt->startMyTimer(triggeringTime);
    }

    //Return
    this->setVisible(true);
}

int HypCamApp::takeRemoteSnapshot( QString fileDestiny, bool squareArea )
{

    //
    // Get cam resolution
    //
    camRes = getCamRes(mainRaspcamSettings->CameraMp);

    //
    //Generates Camera Command
    //..
    structRaspistillCommand* structRaspiCommand = (structRaspistillCommand*)malloc(sizeof(structRaspistillCommand));
    strReqImg *reqImg                           = (strReqImg*)malloc(sizeof(strReqImg));
    memset(reqImg,'\0',sizeof(strReqImg));
    memset(structRaspiCommand,'\0',sizeof(structRaspistillCommand));
    structRaspiCommand->idMsg                   = (unsigned char)4;
    reqImg->squApert                            = squareArea;
    reqImg->raspSett                            = funcFillSnapshotSettings( reqImg->raspSett );
    reqImg->imgCols                             = camRes->width;//2592 | 640
    reqImg->imgRows                             = camRes->height;//1944 | 480
    reqImg->raspSett.Flipped                    = (mainRaspcamSettings->Flipped==1)?1:0;

    //--------------------------------------
    //Create Command
    //--------------------------------------
    QString tmpCommand;
    tmpCommand = genCommand(reqImg, fileDestiny.toStdString())->c_str();

    //--------------------------------------
    //Take Remote Photo
    //--------------------------------------
    bool executedCommand;
    funcRemoteTerminalCommand(tmpCommand.toStdString(),camSelected,0,false,&executedCommand);
    if( !executedCommand )
    {
        funcShowMsgERROR_Timeout("Applying Remote Snapshot Command");
        return -1;
    }

    return 1;
}

structRaspcamSettings HypCamApp::funcFillSnapshotSettings( structRaspcamSettings raspSett )
{
    //Take settings from gui ;D
    //raspSett.width                 = ui->slideWidth->value();
    //raspSett.height                = ui->slideHeight->value();
    memcpy(
                raspSett.AWB,
                mainRaspcamSettings->AWB,
                sizeof(raspSett.AWB)
          );
    //raspSett.Brightness            = ui->slideBrightness->value();
    //raspSett.Contrast              = ui->slideContrast->value();
    memcpy(
                raspSett.Exposure,
                mainRaspcamSettings->Exposure,
                sizeof(raspSett.Exposure)
          );
    //raspSett.ExposureCompensation     = ui->slideExpComp->value();
    //raspSett.Format                   = ( ui->rbFormat1->isChecked() )?1:2;
    //raspSett.Green                    = ui->slideGreen->value();
    raspSett.ISO                        = mainRaspcamSettings->ISO;
    //raspSett.Red                      = ui->slideRed->value();
    //raspSett.Saturation               = ui->slideSaturation->value();
    //raspSett.Sharpness                = ui->slideSharpness->value();
    raspSett.ShutterSpeedMs             = mainRaspcamSettings->ShutterSpeedMs;
    raspSett.SquareShutterSpeedMs       = mainRaspcamSettings->SquareShutterSpeedMs;
    raspSett.Denoise                    = mainRaspcamSettings->Denoise;
    raspSett.ColorBalance               = mainRaspcamSettings->ColorBalance;
    raspSett.TriggeringTimeSecs         = mainRaspcamSettings->TriggeringTimeSecs;

    return raspSett;
}

void HypCamApp::on_pbExit_clicked()
{
    if( funcShowMsgYesNo("ALERT","Do you really want to EXIT?") == 1 )
    {
        QGuiApplication::quit();
    }
}

void HypCamApp::on_pbSynSettings_clicked()
{
    if( obtainFile( _PATH_REMOTE_SLIDESETTINGS, _PATH_LOCAL_SLIDESETTINGS ) )
        funcShowMsgSUCCESS_Timeout("Synchronization Successfully");
    else
        funcShowMsgERROR_Timeout("Sorry, Synchronization not Completed");
}


int HypCamApp::obtainFile(std::string remoteFile, std::string localFile )
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

u_int8_t* HypCamApp::funcQtReceiveFile( std::string fileNameRequested, int* fileLen )
{

    //It assumes that file exists and this was verified by command 10
    //
    //Receive the path of a file into raspCamera and
    //return the file if exists or empty arry otherwise
    *fileLen = 0;

    //Connect to socket
    int socketID = connectSocket( camSelected );
    if( socketID == -1 )
    {
        funcShowMsgERROR_Timeout("ERROR connecting to socket");
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
        funcShowMsgERROR_Timeout("ERROR writing to socket");
        return (u_int8_t*)NULL;
    }

    //Receive file's size
    *fileLen = funcReceiveOnePositiveInteger( socketID );
    if( *fileLen < 1 )
        funcShowMsgERROR_Timeout("*fileLen = " + QString::number(*fileLen));    

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


}

#include "formlstcamactions.h"
#include "ui_formlstcamactions.h"

#include <QMessageBox>
#include <__common.h>
#include <hypCamAPI.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <formsettings.h>
#include <QTimer>
#include <rasphypcam.h>
#include <formtimertxt.h>
#include <hypcamapp.h>

//structRaspcamSettings *mainRaspcamSettings = (structRaspcamSettings*)malloc(sizeof(structRaspcamSettings));
HypCamApp* lstActionsParent;
formLstCamActions::formLstCamActions(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::formLstCamActions)
{
    ui->setupUi(this);

    lstActionsParent = qobject_cast<HypCamApp*>(parent);
}

formLstCamActions::~formLstCamActions()
{
    delete ui;
}

void formLstCamActions::on_pbTestConnection_clicked()
{
    //Check if Camera Settings Exists
    //Initialize camera parameters
    if ( !fileExists( _PATH_RASPICAM_SETTINGS ) )
    {
        funcShowMsgERROR("Please, Set and Save Settings Before to Take a Picture.");
        return (void)false;
    }

    //Fill Camera's Data
    structCamSelected *camSelected = (structCamSelected*)malloc(sizeof(structCamSelected));
    fillCameraSelectedDefault(camSelected);

    //Validatin IP
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
                        tmpCamSett,
                        this
                  );
        if( tmpCamSett->idMsg < 1 ){
            funcShowMsg("ERROR","Camera does not respond at "+IP);
        }else{
            funcShowMsg("SUCCESS","Camera Connected Successfully");
        }
    }
}

void formLstCamActions::on_pbTakeSnapshot_clicked()
{
    funcMainCall_GetSnapshot();
}

void formLstCamActions::funcMainCall_GetSnapshot()
{

    lstActionsParent->setVisible(false);
    this->setVisible(false);

    //---------------------------------------------------
    //Get Last Camera Settings
    //---------------------------------------------------
    if( funcGetRaspCamParameters() != _OK )
    {
        lstActionsParent->setVisible(true);
        this->setVisible(true);
        return (void)false;
    }

    //---------------------------------------------------
    //Get Snapshot-ID Destine
    //---------------------------------------------------
    //DayTime Format
    QString fileName = QDateTime::currentDateTime().toString("ddMMyyyy_HHmmss");
    bool ok;
    fileName = funcGetParam("Snapshot-ID",fileName, &ok);
    if(fileName.isEmpty())
    {
        if(ok==true)
        {
            funcShowMsgERROR_Timeout("Invalid Snapshot-ID",this);
        }
        lstActionsParent->setVisible(false);
        this->setVisible(true);
        return (void)false;
    }

    //---------------------------------------------------
    //Save Last Filename
    //---------------------------------------------------
    saveFile(_FILENAME_LAST_SNAPSHOT,fileName);

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
        funcShowMsgERROR_Timeout("Snapshot-ID Exists: Please, use another", this);
        lstActionsParent->setVisible(true);
        this->setVisible(true);
        return (void)false;
    }

    //--------------------------------------
    //Prepare Remote Scenary
    //--------------------------------------

    //Fill Camera's Data
    structCamSelected *camSelected = (structCamSelected*)malloc(sizeof(structCamSelected));
    fillCameraSelectedDefault(camSelected);

    //Delete Remote File if Exists
    fileName = _PATH_REMOTE_FOLDER_SNAPSHOTS + fileName + _SNAPSHOT_REMOTE_EXTENSION;
    QString tmpCommand;
    tmpCommand.clear();
    tmpCommand.append("sudo rm "+ fileName);
    bool commandExecuted;
    funcRemoteTerminalCommand(tmpCommand.toStdString(),camSelected,0,false,&commandExecuted);
    if( !commandExecuted )
    {
        funcShowMsgERROR_Timeout("Deleting Remote Snapshot-ID", this);
        lstActionsParent->setVisible(true);
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
    if( !takeRemoteSnapshot(fileName, false, mainRaspcamSettings, this) )
    {
        funcShowMsgERROR_Timeout("Taking Full Area Snapshot", this);
        lstActionsParent->setVisible(true);
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
    lstActionsParent->setVisible(true);
    this->setVisible(true);
}

int formLstCamActions::funcValidateFileDirNameDuplicated(QString remoteFile, QString localFile)
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

QString formLstCamActions::funcGetSyncFolder()
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

int formLstCamActions::funcGetRaspCamParameters()
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
        funcShowMsg_Timeout("ALERT: It is your first time...","Please, It is necessary to set and save settings.", this);
        return _ERROR;
    }

    return _OK;
}

void formLstCamActions::on_pbVideoRecording_clicked()
{
    QString remoteVideoID;
    funcMainCall_RecordVideo(&remoteVideoID, this, lstActionsParent);
}

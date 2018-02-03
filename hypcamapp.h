#ifndef HYPCAMAPP_H
#define HYPCAMAPP_H

#include <QMainWindow>
#include <lstStructs.h>

namespace Ui {
class HypCamApp;
}

class HypCamApp : public QMainWindow
{
    Q_OBJECT

public:
    explicit HypCamApp(QWidget *parent = 0);
    ~HypCamApp();

private slots:
    void on_pbCamConn_clicked();

    void on_pbShutdown_clicked();

    std::string funcRemoteTerminalCommand(
                                                std::string command,
                                                structCamSelected *camSelected,
                                                int trigeredTime,
                                                bool waitForAnswer,
                                                bool* ok
                                         );

    /*
    bool funcReceiveFile(
                            int sockfd,
                            unsigned int fileLen,
                            unsigned char *bufferRead,
                            unsigned char *tmpFile
                        );*/

    void on_pbSettings_clicked();

    void funcShowMsg_Timeout(QString title, QString msg, int ms);
    void funcShowMsg_Timeout(QString title, QString msg);

    void funcShowMsgSUCCESS_Timeout(QString msg, int ms);
    void funcShowMsgSUCCESS_Timeout(QString msg);

    void funcShowMsgERROR_Timeout(QString msg, int ms);
    void funcShowMsgERROR_Timeout(QString msg);

    void on_pbVideo_clicked();

    void funcMainCall_RecordVideo(QString* videoID, bool defaultPath=false, bool ROI=true);

    int funcGetRaspCamParameters();

    int funcValidateFileDirNameDuplicated(QString remoteFile, QString localFile);

    QString funcGetSyncFolder();

    cameraResolution* getCamRes(int camResSetting);

    QString genRemoteVideoCommand(structRaspcamSettings *raspcamSettings, QString remoteVideo, bool ROI=true);

    int funcDisplayTimer(QString title, int timeMs, QColor color);

    void on_pbCreateFile_clicked();

    void on_pbSnapshot_clicked();

    void funcMainCall_GetSnapshot();

    int takeRemoteSnapshot( QString fileDestiny, bool squareArea );

    structRaspcamSettings funcFillSnapshotSettings( structRaspcamSettings raspSett );

    void on_pbExit_clicked();

    void on_pbSynSettings_clicked();

    //int obtainFile(std::string remoteFile, std::string localFile );

    //u_int8_t* funcQtReceiveFile(std::string fileNameRequested, int* fileLen );

    void on_pbImgEdition_clicked();

    void on_pbNDVISettings_clicked();

private:
    Ui::HypCamApp *ui;
};

#endif // HYPCAMAPP_H

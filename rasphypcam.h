#ifndef RASPHYPCAM_H
#define RASPHYPCAM_H

#include <string>
#include <iostream>

#include <lstStructs.h>

#include <QProgressBar>
//Set Camera Default Parameters
//structCamSelected *camSelected = (structCamSelected*)malloc(sizeof(structCamSelected));
//structRaspcamSettings *mainRaspcamSettings = (structRaspcamSettings*)malloc(sizeof(structRaspcamSettings));
static structRaspcamSettings *mainRaspcamSettings = (structRaspcamSettings*)malloc(sizeof(structRaspcamSettings));

bool funcReceiveFile(
                        int sockfd,
                        unsigned int fileLen,
                        unsigned char *bufferRead,
                        unsigned char *tmpFile
                    );

void fillCameraSelectedDefault(structCamSelected* camSelected);

void debugMsg( std::string msg );

int readParamFromFile( std::string fileName, std::string* stringContain );

int funcRaspSocketConnect();

int funcRaspFileExists( std::string fileName );
int funcRaspFileExists( int socketID, std::string fileName );

u_int8_t* funcRaspReceiveFile(std::string fileNameRequested , int* fileLen);

bool funcRaspIsIP( std::string ipCandidate );

int funcReceiveOnePositiveInteger(int sockfd);

std::string *genCommand(strReqImg *reqImg, const std::string& fileName);

std::string funcRemoteTerminalCommand(
                                            std::string command,
                                            structCamSelected *camSelected,
                                            int trigeredTime,
                                            bool waitForAnswer,
                                            bool* ok
);

int takeRemoteSnapshot( QString fileDestiny, bool squareArea, structRaspcamSettings* mainRaspcamSettings, QWidget* parent );

structRaspcamSettings funcFillSnapshotSettings( structRaspcamSettings raspSett, structRaspcamSettings* mainRaspcamSettings );

cameraResolution* getCamRes( int camResSetting = 1 );

int obtainFile(std::string remoteFile, std::string localFile, QProgressBar* progressBar );
u_int8_t* funcQtReceiveFile(std::string fileNameRequested, int* fileLen , QProgressBar *progressBar);
void funcShowMsg_Timeout(QString title, QString msg, QWidget* parent, int ms=2000);
//void funcShowMsg_Timeout(QString title, QString msg, QWidget* parent);

void funcShowMsgSUCCESS_Timeout(QString msg, QWidget* parent, int ms=2000);
//void funcShowMsgSUCCESS_Timeout(QString msg, QWidget* parent);

void funcShowMsgERROR_Timeout(QString msg, QWidget* parent, int ms=2000);
//void funcShowMsgERROR_Timeout(QString msg, QWidget* parent);

void funcMainCall_RecordVideo(QString* videoID, QWidget* parent, bool defaultPath=false, bool ROI=true);

int funcValidateFileDirNameDuplicated(QString remoteFile, QString localFile);

QString funcGetSyncFolder();

int funcGetRaspCamParameters(QWidget *parent);

QString genRemoteVideoCommand(structRaspcamSettings *raspcamSettings, QString remoteVideo, bool ROI=true);

int funcDisplayTimer(QString title, int timeMs, QColor color, QWidget *parent);

#endif // RASPHYPCAM_H

#ifndef RASPHYPCAM_H
#define RASPHYPCAM_H

#include <string>
#include <iostream>

#include <lstStructs.h>

#include <QProgressBar>
//Set Camera Default Parameters
//structCamSelected *camSelected = (structCamSelected*)malloc(sizeof(structCamSelected));

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

int obtainFile(std::string remoteFile, std::string localFile, QProgressBar* progressBar );
u_int8_t* funcQtReceiveFile(std::string fileNameRequested, int* fileLen , QProgressBar *progressBar);


#endif // RASPHYPCAM_H

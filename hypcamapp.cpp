#include "hypcamapp.h"
#include "ui_hypcamapp.h"

#include <QMessageBox>
#include <__common.h>
#include <hypCamAPI.h>
#include <arpa/inet.h>
#include <unistd.h>
//#include <QtNetwork/QTcpSocket>

//structSettings *lstSettings = (structSettings*)malloc(sizeof(structSettings));
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

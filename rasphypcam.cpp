#include <rasphypcam.h>

#include <unistd.h>
#include <ifaddrs.h>
#include <netdb.h>


#include <string.h>
//#include <fstream>

#include <math.h>

#include <lstStructs.h>
#include <sstream>
#include <fstream>

#include <__common.h>
#include <QProgressBar>
#include <QMessageBox>
#include <QTimer>
#include <formtimertxt.h>

bool funcReceiveFile(
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

void fillCameraSelectedDefault(structCamSelected* camSelected)
{
    camSelected->isConnected = true;
    camSelected->On = true;
    camSelected->tcpPort = 51717;
    memset(camSelected->IP,'\0',15);
    memcpy(camSelected->IP,"172.24.1.1",strlen("172.24.1.1"));
}

u_int8_t* funcRaspReceiveFile( std::string fileNameRequested, int* fileLen )
{
    //It assumes that file exists and this was verified by command 10
    //
    //Receive the path of a file into raspCamera and
    //return the file if exists or empty arry otherwise

    //Connect to socket
    int socketID = funcRaspSocketConnect();
    if( socketID == -1 )
    {
        debugMsg("ERROR connecting to socket");
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
        debugMsg("ERROR writing to socket");
        return (u_int8_t*)NULL;
    }

    //Receive file's size
    *fileLen = funcReceiveOnePositiveInteger( socketID );
    std::cout << "fileLen: " << *fileLen << std::endl;
    fflush(stdout);

    //Prepare container
    u_int8_t* theFILE = (u_int8_t*)malloc(*fileLen);
    bzero( &theFILE[0], *fileLen );

    //Read file from system buffer
    int filePos     = 0;
    int remainder   = *fileLen;

    //Read image from buffer
    while( remainder > 0 )
    {
        n = read(socketID,(void*)&theFILE[filePos],remainder+1);
        remainder   -= n;
        filePos     += n;

        //std::cout << ( ( 1.0 - ( (float)remainder/(float)*fileLen ) ) * 100.0) << "%" << std::endl;

    }
    //std::cout << "(Last info) filePos: " << filePos << " remainder: " << remainder << " n: " << n << std::endl;
    //fflush(stdout);

    //Close connection
    ::close(socketID);

    //Return file
    return theFILE;


}

int funcRaspFileExists( std::string fileName )
{
    //Ask to the HypCam if file exists
    //RETURN
    //          -1:error during the process | 0:File does not exists | 1:file exists



    //Connect to socket
    int socketID = funcRaspSocketConnect();
    if( socketID == -1 )
    {
        debugMsg("ERROR connecting to socket");
        return -1;
    }

    //
    //Check if file exist
    //

    //Prepare request
    int n;
    strReqFileInfo *reqFileInfo = (strReqFileInfo*)malloc(sizeof(strReqFileInfo));
    memset( reqFileInfo, '\0', sizeof(strReqFileInfo) );

    reqFileInfo->idMsg          = 10;
    reqFileInfo->fileNameLen    = fileName.size();

    memcpy( &reqFileInfo->fileName[0], fileName.c_str(), fileName.size() );

    //printf("Size(%d) Filename: %s\n",fileName.size(),reqFileInfo->fileName);
    //fflush(stdout);

    //Send request
    int tmpFrameLen = sizeof(strReqFileInfo);//IdMsg + lenInfo + fileLen + padding
    n = ::write(socketID,reqFileInfo,tmpFrameLen+1);
    if (n < 0){
        debugMsg("ERROR writing to socket");
        return -1;
    }

    //Receive response
    u_int8_t buffer[frameBodyLen];
    bzero(buffer,frameBodyLen);
    n = read( socketID, buffer, frameBodyLen );
    if (n < 0)
    {
        debugMsg("ERROR reading from socket");
        return -1;
    }

    //Close socket connection
    ::close(socketID);

    //Compute response
    if( buffer[0] == 0 )
        return 0;//File does not exists
    if( buffer[0] == 1 )
        return 1;//File exists

    debugMsg("ERROR unknow code response");
    return -1;//I do not know
}

void debugMsg( std::string msg )
{
    std::cout << msg << std::endl;
    fflush(stdout);
}

int readParamFromFile( std::string fileName, std::string* stringContain )
{
    //Read a file an put contain into stringContain
    //Return: -1 if error reading file | 0 if file is empty | 1 file is not empty

    stringContain->assign("");

    //Read file
    std::ifstream inputFile( fileName.c_str() );
    std::string line;
    if( !inputFile )
    {
        debugMsg("ERROR reading file");
        return 1;
    }
    else
    {
        inputFile >> line;
        stringContain->append(line);
    }
    inputFile.close();

    //Check if is empty
    if( stringContain->empty() )
    {
        return 0;
    }
    return 1;
}

int funcRaspSocketConnect()
{
    //Connect to socket
    //RETURN
    // -1: if error | socket: if connection success

    int PORT;
    std::string* IP = new std::string();
    std::string* fileBuffer = new std::string();

    //Read IP
    if( readParamFromFile( _PATH_LAST_IP, fileBuffer ) == 1 )
        IP->append( fileBuffer->c_str() );
    else
    {
        debugMsg( "ERROR reading IP or IP corrupted" );
        return -1;
    }

    //Read PORT
    if( readParamFromFile( _PATH_PORT_TO_CONNECT, fileBuffer ) == 1 )
        PORT = (int)std::strtol( (char*)fileBuffer->c_str(), (char **)NULL, 10 );
    else
    {
        debugMsg( "ERROR reading IP or IP corrupted" );
        return -1;
    }



    //Prepare command message
    int sockfd = -1;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    std::cout << "Connecting to IP:" << IP->c_str() << " PORT: " << PORT << std::endl;

    if (sockfd < 0){
        std::cout << "ERROR opening socket IP:" << IP << " PORT: " << PORT << std::endl;
        return -1;
    }

    server = gethostbyname( (char*)IP->c_str() );
    if (server == NULL)
    {
        std::cout << "ERROR Socket, no such host:" << std::endl;
        return -1;
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
        (char *)&serv_addr.sin_addr.s_addr,
        server->h_length);
    serv_addr.sin_port = htons( PORT );

    if (::connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
    {
        std::cout << "ERROR connecting to IP:" << IP << " PORT: " << PORT << std::endl;
        return -1;
    }

    return sockfd;
}


bool funcRaspIsIP( std::string ipCandidate ){
    std::string delimiter = ".";
    ipCandidate.append(delimiter);
    size_t pos = 0;
    std::string token;
    int ipVal;
    char *pEnd;
    u_int8_t numElems = 0;
    while ((pos = ipCandidate.find(delimiter)) != std::string::npos) {
        token = ipCandidate.substr(0, pos);
        if( !token.empty() ){
            ipVal = strtol ( token.c_str(), &pEnd, 10 );
            //funcShowMsg("IP",QString::number(ipVal));
            if( ipVal < 0 || ipVal > 255 ){
                return false;
            }
            numElems++;
        }
        ipCandidate.erase(0, pos + delimiter.length());
    }
    if( numElems < 4 ){
        return false;
    }else{
        if( strtol ( token.c_str(), &pEnd, 10 ) == 254 ){
            return false;
        }else{
            return true;
        }
    }
}


int funcReceiveOnePositiveInteger( int sockfd )
{
    //qDebug() << "funcReceiveOnePositiveInteger";
    //It assumess that frame has memory prviously allocated
    //Return the number n of bytes received

    u_int8_t buffer[sizeof(int)+1];

    int n;
    //qDebug() << "READING";
    n = read(sockfd,buffer,sizeof(int)+1);
    if( n < 0 )
        return -1;//Error receiving message

    memcpy(&n,buffer,sizeof(int));

    return n;
}


std::string *genCommand(strReqImg *reqImg, const std::string& fileName)
{

    //Initialize command
    //..
    std::string *tmpCommand = new std::string("raspistill -o ");
    std::ostringstream ss;
    tmpCommand->append(fileName);
    tmpCommand->append(" -n -q 100 -gc");

    //Colour balance?
    if(reqImg->raspSett.ColorBalance){
        tmpCommand->append(" -ifx colourbalance");
    }

    //Denoise?
    if(reqImg->raspSett.Denoise){
        tmpCommand->append(" -ifx denoise");
    }

    //Square Shuter speed
    int shutSpeed = reqImg->raspSett.SquareShutterSpeedMs;
    if( (reqImg->squApert && shutSpeed>0))
    {
        ss.str("");
        ss<<shutSpeed;
        tmpCommand->append(" -ss " + ss.str());
    }

    //Diffraction Shuter speed
    shutSpeed = reqImg->raspSett.ShutterSpeedMs;
    if(
        (!reqImg->squApert && shutSpeed>0) ||	//Whe is by parts
        (reqImg->fullFrame  && shutSpeed>0)	//Whe is unique and shutter speed has been setted
    )
    {
        ss.str("");
        ss<<shutSpeed;
        tmpCommand->append(" -ss " + ss.str());
    }

    //Trigering timer
    if( reqImg->raspSett.TriggeringTimeSecs > 0 )
    {
        ss.str("");
        ss<<(reqImg->raspSett.TriggeringTimeSecs*1000);
        tmpCommand->append(" -t " + ss.str());
    }
    else
    {
        ss.str("");
        ss<<250;//Milliseconds by default
        tmpCommand->append(" -t " + ss.str());
    }

    //Width
    ss.str("");
    ss<<reqImg->imgCols;
    tmpCommand->append(" -w " + ss.str());

    //Height
    ss.str("");
    ss<<reqImg->imgRows;
    tmpCommand->append(" -h " + ss.str());

    //AWB
    std::string sAWB((char*)reqImg->raspSett.AWB, sizeof(reqImg->raspSett.AWB));
    if( strcmp(sAWB.c_str(),"none") != 0 ){
        tmpCommand->append(" -awb ");
        tmpCommand->append(sAWB.c_str());
        //printf("Entro a AWB: %s\n",sAWB.c_str());
    }

    //Exposure
    std::string sExposure((char*)reqImg->raspSett.Exposure, sizeof(reqImg->raspSett.Exposure));
    if( strcmp(sExposure.c_str(),"none") != 0 ){
        tmpCommand->append(" -ex ");
        tmpCommand->append(sExposure.c_str());
        //printf("Entro a Exp: %s\n",sExposure.c_str());
    }

    //ISO
    if( reqImg->raspSett.ISO > 0 ){
        ss.str("");
        ss<<reqImg->raspSett.ISO;
        tmpCommand->append(" -ISO " + ss.str());
    }

    //FLIPPED
    if( reqImg->raspSett.Flipped ){
        tmpCommand->append(" -vf ");
    }


    return tmpCommand;
}

int obtainFile(std::string remoteFile, std::string localFile, QProgressBar* progressBar )
{
    if( funcRaspFileExists( remoteFile ) == 0 )
    {
        debugMsg("File does not exists");
        return _ERROR;
    }
    else
    {
        int fileLen = 0;
        u_int8_t* fileReceived = funcQtReceiveFile( remoteFile, &fileLen, progressBar );
        if(fileLen==-1)
        {
            funcShowMsgERROR("Receiving File");
        }
        else
        {
            //funcShowMsg_Timeout("",QString::fromStdString(remoteFile));

            //funcShowMsg_Timeout("fileLen",QString::number(fileLen));

            //funcShowMsg("fileLen",QString::number(fileLen));

            if( saveBinFile_From_u_int8_T(localFile,fileReceived,fileLen) )
                return _OK;
            else
                return _ERROR;
        }
    }
    return _OK;
}

u_int8_t* funcQtReceiveFile( std::string fileNameRequested, int* fileLen, QProgressBar* progressBar )
{
    //It assumes that file exists and this was verified by command 10
    //
    //Receive the path of a file into raspCamera and
    //return the file if exists or empty arry otherwise
    *fileLen = 0;

    //Fill Camera's Data
    structCamSelected *camSelected = (structCamSelected*)malloc(sizeof(structCamSelected));
    fillCameraSelectedDefault(camSelected);

    //Connect to socket
    int socketID = connectSocket( camSelected );
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
        return (u_int8_t*)NULL;
    }

    //Prepare container
    u_int8_t* theFILE = (u_int8_t*)malloc(*fileLen);
    bzero( &theFILE[0], *fileLen );

    //Read file from system buffer
    int filePos     = 0;
    int remainder   = *fileLen;

    //Read image from buffer
    int aux;
    while( remainder > 0 )
    {
        n = read(socketID,(void*)&theFILE[filePos],remainder);
        remainder   -= n;
        filePos     += n;

        //Refresh Progress Bar
        aux = round( ((float)(*fileLen-remainder)/(float)*fileLen )*100.0 );
        progressBar->setValue(aux);
        progressBar->update();
        QtDelay(1);
    }

    //Refresh Progress Bar
    progressBar->setVisible(false);
    progressBar->setValue( 0 );
    progressBar->update();
    QtDelay(1);

    //Close connection
    ::close(socketID);

    //Return file
    return theFILE;


}

std::string funcRemoteTerminalCommand(
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

int takeRemoteSnapshot(
                            QString fileDestiny,
                            bool squareArea,
                            structRaspcamSettings* mainRaspcamSettings,
                            QWidget* parent
){

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
    reqImg->raspSett                            = funcFillSnapshotSettings( reqImg->raspSett, mainRaspcamSettings );
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

    //Fill Camera's Data
    structCamSelected *camSelected = (structCamSelected*)malloc(sizeof(structCamSelected));
    fillCameraSelectedDefault(camSelected);

    //
    bool executedCommand;
    funcRemoteTerminalCommand(tmpCommand.toStdString(),camSelected,0,false,&executedCommand);
    if( !executedCommand )
    {
        funcShowMsgERROR_Timeout("Applying Remote Snapshot Command", parent);
        return -1;
    }

    return 1;
}

structRaspcamSettings funcFillSnapshotSettings( structRaspcamSettings raspSett, structRaspcamSettings* mainRaspcamSettings )
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

cameraResolution* getCamRes( int camResSetting )
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


void funcShowMsg_Timeout(QString title, QString msg, QWidget* parent, int ms)
{
    QMessageBox *msgBox         = new QMessageBox(QMessageBox::Warning,title,msg,NULL);
    QTimer *msgBoxCloseTimer    = new QTimer(parent);
    msgBoxCloseTimer->setInterval(ms);
    msgBoxCloseTimer->setSingleShot(true);
    QObject::connect(msgBoxCloseTimer, SIGNAL(timeout()), msgBox, SLOT(reject()));
    msgBoxCloseTimer->start();
    msgBox->exec();
}

void funcShowMsgSUCCESS_Timeout(QString msg, QWidget* parent, int ms)
{
    QMessageBox *msgBox         = new QMessageBox(QMessageBox::Warning,"SUCCESS",msg,NULL);
    QTimer *msgBoxCloseTimer    = new QTimer(parent);
    msgBoxCloseTimer->setInterval(ms);
    msgBoxCloseTimer->setSingleShot(true);
    QObject::connect(msgBoxCloseTimer, SIGNAL(timeout()), msgBox, SLOT(reject()));
    msgBoxCloseTimer->start();
    msgBox->exec();
}

void funcShowMsgERROR_Timeout(QString msg, QWidget* parent, int ms)
{
    QMessageBox *msgBox         = new QMessageBox(QMessageBox::Warning,"ERROR",msg,NULL);
    QTimer *msgBoxCloseTimer    = new QTimer(parent);
    msgBoxCloseTimer->setInterval(ms);
    msgBoxCloseTimer->setSingleShot(true);
    QObject::connect(msgBoxCloseTimer, SIGNAL(timeout()), msgBox, SLOT(reject()));
    msgBoxCloseTimer->start();
    msgBox->exec();
}

void funcMainCall_RecordVideo(QString* videoID, QWidget* parent, QWidget* grandpa, bool defaultPath, bool ROI)
{
    bool commandExecuted;

    videoID->clear();

    //---------------------------------------------------
    //Get Last Camera Settings
    //---------------------------------------------------
    if( funcGetRaspCamParameters(parent) != _OK )
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
        grandpa->setVisible(false);
        parent->setVisible(false);

        QString fileName = QDateTime::currentDateTime().toString("ddMMyyyy_HHmmss");
        bool ok;
        fileName = funcGetParam("Video-ID",fileName, &ok);
        if(videoID->isEmpty())
        {
            if(ok==true)
            {
                funcShowMsgERROR_Timeout("Invalid Video-ID",parent);
            }
            grandpa->setVisible(true);
            parent->setVisible(true);
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
                funcShowMsgERROR_Timeout("Video-ID Exists Locally: Please, use another", parent);
            else
                funcShowMsgERROR_Timeout("Video-ID Exists Remotelly: Please, use another", parent);
            grandpa->setVisible(true);
            parent->setVisible(true);
            return (void)false;
        }

        //---------------------------------------------------
        //Prepare Remote Scenary
        //---------------------------------------------------

        //Fill Camera's Data
        structCamSelected *camSelected = (structCamSelected*)malloc(sizeof(structCamSelected));
        fillCameraSelectedDefault(camSelected);

        //Delete Remote File if Exists
        *videoID = _PATH_REMOTE_FOLDER_VIDEOS + *videoID + _VIDEO_EXTENSION;
        QString tmpCommand;
        tmpCommand.clear();
        tmpCommand.append("sudo rm "+ *videoID);
        funcRemoteTerminalCommand(tmpCommand.toStdString(),camSelected,0,false,&commandExecuted);
        if( !commandExecuted )
        {
            funcShowMsgERROR_Timeout("Deleting Remote videoID",parent);
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

    //Fill Camera's Data
    structCamSelected *camSelected = (structCamSelected*)malloc(sizeof(structCamSelected));
    fillCameraSelectedDefault(camSelected);


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
        funcShowMsgERROR_Timeout("Starting Remote Recording", parent);
        return (void)false;
    }

    //-----------------------------------------------------
    //Display Timer
    //-----------------------------------------------------
    //Before to Start Recording
    funcDisplayTimer("Countdown to Recording...",mainRaspcamSettings->TriggeringTimeSecs,Qt::black, parent);

    //During Recording
    funcDisplayTimer("Recording...",mainRaspcamSettings->VideoDurationSecs,Qt::red,parent);

    //Return
    grandpa->setVisible(true);
    parent->setVisible(true);

}

int funcDisplayTimer(QString title, int timeMs, QColor color, QWidget* parent)
{
    if( timeMs > 0 )
    {
        formTimerTxt* timerTxt = new formTimerTxt(parent,title,timeMs,color);
        timerTxt->setModal(true);
        timerTxt->show();
        QtDelay(200);
        timerTxt->startMyTimer(timeMs);
    }
    return 1;
}

int funcValidateFileDirNameDuplicated(QString remoteFile, QString localFile)
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

QString funcGetSyncFolder()
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

int funcGetRaspCamParameters(QWidget* parent)
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
        funcShowMsg_Timeout("ALERT: It is your first time...","Please, It is necessary to set and save settings.",parent);
        return _ERROR;
    }

    return _OK;
}

QString genRemoteVideoCommand(structRaspcamSettings* raspcamSettings, QString remoteVideo, bool ROI)
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

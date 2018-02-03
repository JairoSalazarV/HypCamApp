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

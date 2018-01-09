#include "__common.h"

#include <QMessageBox>
#include <QFile>
#include <QXmlStreamReader>

#include <ifaddrs.h>
#include <netdb.h>

#include <QFile>
#include <QTextStream>

#include <QFileInfo>

//#include <customQMatrix3x3.h>
//#include <customQMatrix4x3.h>

#include <QDesktopServices>
#include <QFileDialog>

//#include <iostream>
//#include <fstream>

#include <QPixmap>
//#include <graphicsview.h>
#include <QApplication>
#include <QDesktopWidget>

#include <lstcustoms.h>

#include <QInputDialog>

#include <QFileDialog>
#include <QImage>



int funcDeleteFile( QString fileName )
{
    QFile tmpFile( fileName );
    if( tmpFile.exists() )
    {
        tmpFile.remove();
        return 1;//File exists and removed
    }
    else
    {
        return 2;//File does not exists but is equal to be removed
    }
    return -1;//Error
}

QString funcRemoveFileNameFromPath( QString Path ){
    return QFileInfo(Path).absolutePath();
}


QString timeToQString( unsigned int totMilli )
{
    unsigned int h          = round(totMilli/(1000*60*60));
    totMilli                = totMilli - (h*(1000*60*60));
    unsigned int m          = round(totMilli/(1000*60));
    totMilli                = totMilli - (m*(1000*60));
    unsigned int s          = round(totMilli/1000);
    unsigned int u          = totMilli - (s*1000);
    QString timeElapsed     = "HH:MM:SS:U( ";
    timeElapsed            += (h>9)?QString::number(h)+":":"0"+QString::number(h)+":";
    timeElapsed            += (m>9)?QString::number(m)+":":"0"+QString::number(m)+":";
    timeElapsed            += (s>9)?QString::number(s)+":":"0"+QString::number(s)+":";
    timeElapsed            += QString::number(u);
    timeElapsed            += " )";
    return timeElapsed;
}


double vectorMax(double *vector, int len)
{
    int i;
    double max;
    max = 0;
    for(i=0;i<len;i++)
    {
        max = (max<vector[i])?vector[i]:max;
    }
    return max;
}

double vectorMaxQListQString(QList<QString> lst)
{
    int i;
    double max;
    max = 0;
    for(i=0;i<lst.size();i++)
    {
        max = (max<lst.at(i).toDouble())?lst.at(i).toDouble():max;
    }
    return max;
}

QImage bilinearInterpolationQImage(QImage img)
{
    QImage aux;
    aux = img;
    int x, y;
    QColor tmpPixel;
    QRgb Q11, Q12, Q21, Q22;
    int r, g, b;
    for(y=1; y<img.height()-1; y++)
    {
        for(x=1; x<img.width()-1; x++)
        {
            Q11 = img.pixel(x-1,y-1);
            Q12 = img.pixel(x+1,y-1);
            Q21 = img.pixel(x-1,y+1);
            Q22 = img.pixel(x+1,y+1);
            r = round( (qRed(Q11)+qRed(Q12)+qRed(Q21)+qRed(Q22)/4.0) );
            g = round( (qGreen(Q11)+qGreen(Q12)+qGreen(Q21)+qGreen(Q22)/4.0) );
            b = round( (qBlue(Q11)+qBlue(Q12)+qBlue(Q21)+qBlue(Q22)/4.0) );
            tmpPixel.setRed(r);
            tmpPixel.setGreen(g);
            tmpPixel.setBlue(b);
            aux.setPixel( x, y, tmpPixel.rgba() );
        }
    }
    return aux;
}



void funcQStringToSensitivities(QString txt, QList<double> *p)
{
    int i;
    QList<QString> lstSensitivities;
    lstSensitivities = txt.split(",");
    //p = (double*)malloc(lstSensitivities.count()*sizeof(double));
    for(i=0; i<lstSensitivities.count(); i++)
    {
        p->append( lstSensitivities.at(i).toDouble(0) );
    }
}

void funcTransPix( calcAndCropSnap *calStruct, int w, int h, int W, int H ){
    //Extrapolate dimensions
    calStruct->X1   = round( ((float)W/(float)w)*(float)calStruct->x1 );
    calStruct->Y1   = round( ((float)H/(float)h)*(float)calStruct->y1 );
    calStruct->X2   = round( ((float)W/(float)w)*(float)calStruct->x2 );
    calStruct->Y2   = round( ((float)H/(float)h)*(float)calStruct->y2 );
    calStruct->lenW = abs(calStruct->X2-calStruct->X1);
    calStruct->lenH = abs(calStruct->Y2-calStruct->Y1);
}

colorAnalyseResult *funcAnalizeImage( QImage *img ){

    //Reserve memory
    //..
    colorAnalyseResult* colResults = (colorAnalyseResult*)malloc(sizeof(colorAnalyseResult));
    int aux = img->width() * sizeof(int);
    colResults->Red = (int*)malloc( aux );
    colResults->Green = (int*)malloc( aux );
    colResults->Blue = (int*)malloc( aux );
    memset( colResults->Red, '\0', aux );
    memset( colResults->Green, '\0', aux );
    memset( colResults->Blue, '\0', aux );

    //Procced to analyze the image
    //..
    int r,c,tmpMaxR=0,tmpMaxG=0,tmpMaxB=0,tmpMaxRx=0,tmpMaxGx=0,tmpMaxBx=0;
    QRgb pixel;
    colResults->maxRx   = 0;
    colResults->maxGx   = 0;
    colResults->maxBx   = 0;
    colResults->maxMax  = 0;
    //colResults->maxR = 0;
    //colResults->maxG = 0;
    //colResults->maxB = 0;
    for(r=0;r<img->height();r++){
        tmpMaxR = 0;
        tmpMaxG = 0;
        tmpMaxB = 0;
        tmpMaxRx = 0;
        tmpMaxGx = 0;
        tmpMaxBx = 0;
        for(c=0;c<img->width();c++){
            if(!img->valid(c,r)){
                qDebug() << "Invalid r: " << r << "c: "<<c;
                qDebug() << "img.width(): " << img->width();
                qDebug() << "img.height(): " << img->height();
                return colResults;
            }
            pixel = img->pixel(c,r);
            //Red
            if(tmpMaxR<qRed(pixel)){
                tmpMaxR     = qRed(pixel);
                tmpMaxRx    = c;
            }
            //Green
            if(tmpMaxG<qGreen(pixel)){
                tmpMaxG     = qGreen(pixel);
                tmpMaxGx    = c;
            }
            //Blue
            if(tmpMaxB<qBlue(pixel)){
                tmpMaxB     = qBlue(pixel);
                tmpMaxBx    = c;
            }
        }
        //Acumulate
        colResults->maxRx += tmpMaxRx;
        colResults->maxGx += tmpMaxGx;
        colResults->maxBx += tmpMaxBx;


    }
    //Mean
    colResults->maxRx = round( (float)colResults->maxRx / (float)img->height());
    colResults->maxGx = round( (float)colResults->maxGx / (float)img->height());
    colResults->maxBx = round( (float)colResults->maxBx / (float)img->height());

    tmpMaxRx = colResults->maxRx;
    tmpMaxGx = colResults->maxGx;
    tmpMaxBx = colResults->maxBx;
    //funcShowMsg("Result",
    //            "(" + QString::number(tmpMaxR)+","+QString::number(tmpMaxRx) + ")" +
    //            "(" + QString::number(tmpMaxG)+","+QString::number(tmpMaxGx) + ")" +
    //            "(" + QString::number(tmpMaxB)+","+QString::number(tmpMaxBx) + ")"
    //           );

    //Get the maxMax
    //..
    if( colResults->maxMax < colResults->maxR ){
        colResults->maxMax      = colResults->maxR;
        colResults->maxMaxx     = colResults->maxRx;
        colResults->maxMaxColor = 1;
    }
    if( colResults->maxMax < colResults->maxG ){
        colResults->maxMax      = colResults->maxG;
        colResults->maxMaxx     = colResults->maxGx;
        colResults->maxMaxColor = 2;
    }
    if( colResults->maxMax < colResults->maxB ){
        colResults->maxMax      = colResults->maxB;
        colResults->maxMaxx     = colResults->maxBx;
        colResults->maxMaxColor = 3;
    }

    return colResults;



    /*
    //Reserve memory
    //..
    colorAnalyseResult* colResults = (colorAnalyseResult*)malloc(sizeof(colorAnalyseResult));
    int aux = img->width() * sizeof(int);
    colResults->Red = (int*)malloc( aux );
    colResults->Green = (int*)malloc( aux );
    colResults->Blue = (int*)malloc( aux );
    memset( colResults->Red, '\0', aux );
    memset( colResults->Green, '\0', aux );
    memset( colResults->Blue, '\0', aux );

    //Procced to analyze the image
    //..
    int r, c;
    colResults->maxR    = 0;
    colResults->maxG    = 0;
    colResults->maxB    = 0;
    colResults->maxMax  = 0;
    QRgb pixel;
    for(c=0;c<img->width();c++){
        colResults->Red[c]   = 0;
        colResults->Green[c] = 0;
        colResults->Blue[c]  = 0;
        for(r=0;r<img->height();r++){
            if(!img->valid(QPoint(c,r))){
                qDebug() << "Invalid r: " << r << "c: "<<c;
                qDebug() << "img.width(): " << img->width();
                qDebug() << "img.height(): " << img->height();
                break;
                break;
            }
            pixel     = img->pixel(QPoint(c,r));
            colResults->Red[c]   += qRed(pixel);
            colResults->Green[c] += qGreen(pixel);
            colResults->Blue[c]  += qBlue(pixel);
        }
        colResults->Red[c]   = round((float)colResults->Red[c]/(float)img->height());
        colResults->Green[c] = round((float)colResults->Green[c]/(float)img->height());
        colResults->Blue[c]  = round((float)colResults->Blue[c]/(float)img->height());
        if( colResults->Red[c] > colResults->maxR ){
            colResults->maxR        = colResults->Red[c];
            colResults->maxRx       = c;
        }
        if( colResults->Green[c] > colResults->maxG ){
            colResults->maxG        = colResults->Green[c];
            colResults->maxGx       = c;
        }
        if( colResults->Blue[c] > colResults->maxB ){
            colResults->maxB        = colResults->Blue[c];
            colResults->maxBx       = c;
        }
    }

    //Get the maxMax
    //..
    if( colResults->maxMax < colResults->maxR ){
        colResults->maxMax      = colResults->maxR;
        colResults->maxMaxx     = colResults->maxRx;
        colResults->maxMaxColor = 1;
    }
    if( colResults->maxMax < colResults->maxG ){
        colResults->maxMax      = colResults->maxG;
        colResults->maxMaxx     = colResults->maxGx;
        colResults->maxMaxColor = 2;
    }
    if( colResults->maxMax < colResults->maxB ){
        colResults->maxMax      = colResults->maxB;
        colResults->maxMaxx     = colResults->maxBx;
        colResults->maxMaxColor = 3;
    }

    return colResults;
    */
}

/*
IplImage *funcGetImgFromCam( int usb, int stabMs ){
    //Turn on camera
    //..
    CvCapture* usbCam  = cvCreateCameraCapture( usb );
    cvSetCaptureProperty( usbCam,  CV_CAP_PROP_FRAME_WIDTH,  320*_FACT_MULT );
    cvSetCaptureProperty( usbCam,  CV_CAP_PROP_FRAME_HEIGHT, 240*_FACT_MULT );
    assert( usbCam );
    QtDelay(stabMs);

    //Create image
    //..
    IplImage *tmpCam = cvQueryFrame( usbCam );
    if( ( tmpCam = cvQueryFrame( usbCam ) ) ){
        //Get the image
        QString tmpName = "./snapshots/tmpUSB.png";
        if( _USE_CAM ){
            IplImage *imgRot = cvCreateImage(
                                                cvSize(tmpCam->height,tmpCam->width),
                                                tmpCam->depth,
                                                tmpCam->nChannels
                                            );
            cvTranspose(tmpCam,imgRot);
            cvTranspose(tmpCam,imgRot);
            cvTranspose(tmpCam,imgRot);

            //cv::imwrite( tmpName.toStdString(), cv::Mat(imgRot, true) );
            cv::imwrite( tmpName.toStdString(), cv::cvarrToMat(imgRot) );

            cvReleaseCapture(&usbCam);
            return imgRot;
        }else{
            cvReleaseCapture(&usbCam);
            return cvLoadImage(tmpName.toStdString().c_str(), 1);
        }
    }else{
        qDebug() << "ERROR retrieving image from usb(" << usb << ")";
    }
    cvReleaseCapture(&usbCam);
    return tmpCam;
}
*/

bool saveFile( QString fileName, QString contain ){
    QFile file(fileName);
    if(file.exists()){
        if(!file.remove()){
            return false;
        }
    }
    if (file.open(QIODevice::ReadWrite)) {
        QTextStream stream(&file);
        stream << contain << endl;
        file.close();
    }else{
        return false;
    }
    return true;
}

QImage funcRotateImage(QString filePath, float rotAngle){
    QTransform transformation;
    transformation.rotate(rotAngle);
    QImage tmpImg(filePath);
    tmpImg = tmpImg.transformed(transformation);
    return tmpImg;
}

QString readAllFile( QString filePath ){
    QFile tmpFile(filePath);
    if( tmpFile.exists() )
    {
        tmpFile.open(QIODevice::ReadOnly);
        QTextStream tmpStream(&tmpFile);
        return tmpStream.readAll();
    }
    else
    {
        return _ERROR_FILE_NOTEXISTS;
    }
    return _ERROR_FILE;
}

int fileIsValid(QString fileContain)
{
    // return:
    //  1: exists and it is not empty
    // -1: empty file
    // -2: error reading
    // -3: it does note exist
    if( fileContain.isEmpty() )
    {
        return -1;
    }
    if( fileContain.contains(_ERROR_FILE) )
    {
        return -2;
    }
    if( fileContain.contains(_ERROR_FILE_NOTEXISTS) )
    {
        return -3;
    }
    return 1;
}


int readFileParam(QString fileName, QString* tmpFileContain)
{
    *tmpFileContain = "";
    if( fileExists(fileName) )
    {
        *tmpFileContain = readAllFile(fileName);
        if( fileIsValid(*tmpFileContain) )
        {
            *tmpFileContain = tmpFileContain->trimmed();
            tmpFileContain->replace("\n","");
        }
        else
        {
            return _ERROR;
        }
    }else
    {
        saveFile(fileName,*tmpFileContain);
        return _FAILURE;
    }
    return _OK;
}

QString readFileParam(QString fileName){
    QString tmpFileContain = "";
    if( fileExists(fileName) )
    {
        tmpFileContain = readAllFile(fileName);
        if( fileIsValid(tmpFileContain) )
        {
            tmpFileContain = tmpFileContain.trimmed();
            tmpFileContain.replace("\n","");
        }
    }else
    {
        saveFile(fileName,tmpFileContain);
    }
    return tmpFileContain;
}






bool funGetSquareXML( QString fileName, squareAperture *squareParam ){

    QFile *xmlFile = new QFile( fileName );
    if (!xmlFile->open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    QXmlStreamReader *xmlReader = new QXmlStreamReader(xmlFile);


    //Parse the XML until we reach end of it
    while(!xmlReader->atEnd() && !xmlReader->hasError()) {
        // Read next element
        QXmlStreamReader::TokenType token = xmlReader->readNext();
        if(token == QXmlStreamReader::StartDocument) {
            continue;
        }
        //If token is StartElement - read it
        if(token == QXmlStreamReader::StartElement) {

            if( xmlReader->name()=="W" )
                squareParam->canvasW = xmlReader->readElementText().toInt(0);

            if( xmlReader->name()=="H" )
                squareParam->canvasH = xmlReader->readElementText().toInt(0);

            if( xmlReader->name()=="x" )
                squareParam->rectX = xmlReader->readElementText().toInt(0);

            if( xmlReader->name()=="y" )
                squareParam->rectY = xmlReader->readElementText().toInt(0);

            if( xmlReader->name()=="w" )
                squareParam->rectW = xmlReader->readElementText().toInt(0);

            if( xmlReader->name()=="h" )
                squareParam->rectH = xmlReader->readElementText().toInt(0);
        }
    }
    if(xmlReader->hasError()) {
        funcShowMsg("Parse Error",xmlReader->errorString());
        return false;
    }
    xmlReader->clear();
    xmlFile->close();
    return true;
}


bool funGetSlideSettingsXML( QString fileName, strReqImg *reqImg ){

    QFile *xmlFile = new QFile( fileName );
    if (!xmlFile->open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    QXmlStreamReader *xmlReader = new QXmlStreamReader(xmlFile);


    //Parse the XML until we reach end of it
    while(!xmlReader->atEnd() && !xmlReader->hasError()) {
        // Read next element
        QXmlStreamReader::TokenType token = xmlReader->readNext();
        if(token == QXmlStreamReader::StartDocument) {
            continue;
        }
        //If token is StartElement - read it
        if(token == QXmlStreamReader::StartElement) {

            if( xmlReader->name()=="degreeIni" )
                reqImg->slide.degreeIni = xmlReader->readElementText().toInt(0);

            if( xmlReader->name()=="degreeEnd" )
                reqImg->slide.degreeEnd = xmlReader->readElementText().toInt(0);

            if( xmlReader->name()=="degreeJump" )
                reqImg->slide.degreeJump = xmlReader->readElementText().toInt(0);

            if( xmlReader->name()=="speed" )
                reqImg->slide.speed = xmlReader->readElementText().toInt(0);
        }
    }
    if(xmlReader->hasError()) {
        funcShowMsg("Parse Error",xmlReader->errorString());
        return false;
    }
    xmlReader->clear();
    xmlFile->close();
    return true;
}

bool funcGetRaspParamFromXML( structRaspcamSettings *raspcamSettings, QString filePath ){

    QFile *xmlFile = new QFile( filePath );
    if (!xmlFile->open(QIODevice::ReadOnly | QIODevice::Text)) {
        funcShowMsg("ERROR","Loading "+ filePath );
    }
    QXmlStreamReader *xmlReader = new QXmlStreamReader(xmlFile);


    //Parse the XML until we reach end of it
    while(!xmlReader->atEnd() && !xmlReader->hasError()) {
        // Read next element
        QXmlStreamReader::TokenType token = xmlReader->readNext();
        //If token is just StartDocument - go to next
        if(token == QXmlStreamReader::StartDocument) {
                continue;
        }
        //If token is StartElement - read it
        if(token == QXmlStreamReader::StartElement) {
            if( xmlReader->name()=="AWB" ){                
                memcpy(
                            raspcamSettings->AWB,
                            xmlReader->readElementText().toStdString().c_str(),
                            sizeof(raspcamSettings->AWB)
                      );
            }
            if( xmlReader->name()=="Exposure" ){
                memcpy(
                            raspcamSettings->Exposure,
                            xmlReader->readElementText().toStdString().c_str(),
                            sizeof(raspcamSettings->Exposure)
                      );
            }

            if( xmlReader->name()=="VideoDurationSecs" )
                raspcamSettings->VideoDurationSecs = xmlReader->readElementText().toInt(0);
            if( xmlReader->name()=="Brightness" )
                raspcamSettings->Brightness = xmlReader->readElementText().toInt(0);
            if( xmlReader->name()=="Sharpness" )
                raspcamSettings->Sharpness = xmlReader->readElementText().toInt(0);
            if( xmlReader->name()=="Contrast" )
                raspcamSettings->Contrast = xmlReader->readElementText().toInt(0);
            if( xmlReader->name()=="Saturation" )
                raspcamSettings->Saturation = xmlReader->readElementText().toInt(0);
            if( xmlReader->name()=="ShutterSpeedMs" )
                raspcamSettings->ShutterSpeedMs = xmlReader->readElementText().toInt(0);
            if( xmlReader->name()=="SquareShutterSpeedMs" )
                raspcamSettings->SquareShutterSpeedMs = xmlReader->readElementText().toInt(0);
            if( xmlReader->name()=="TimelapseDurationSecs" )
                raspcamSettings->TimelapseDurationSecs = xmlReader->readElementText().toInt(0);
            if( xmlReader->name()=="TimelapseInterval_ms" )
                raspcamSettings->TimelapseInterval_ms = xmlReader->readElementText().toInt(0);
            if( xmlReader->name()=="ISO" )
                raspcamSettings->ISO = xmlReader->readElementText().toInt(0);
            if( xmlReader->name()=="ExposureCompensation" )
                raspcamSettings->ExposureCompensation = xmlReader->readElementText().toInt(0);
            if( xmlReader->name()=="Format" )
                raspcamSettings->Format = xmlReader->readElementText().toInt(0);
            if( xmlReader->name()=="Red" )
                raspcamSettings->Red = xmlReader->readElementText().toInt(0);
            if( xmlReader->name()=="Green" )
                raspcamSettings->Green = xmlReader->readElementText().toInt(0);
            if( xmlReader->name()=="Denoise" )
                raspcamSettings->Denoise = xmlReader->readElementText().toInt(0);
            if( xmlReader->name()=="FullPhoto" )
                raspcamSettings->FullPhoto = xmlReader->readElementText().toInt(0);
            if( xmlReader->name()=="ColorBalance" )
                raspcamSettings->ColorBalance = xmlReader->readElementText().toInt(0);
            if( xmlReader->name()=="Preview" )
                raspcamSettings->Preview = xmlReader->readElementText().toInt(0);
            if( xmlReader->name()=="OneShot" )
                raspcamSettings->OneShot = xmlReader->readElementText().toInt(0);
            if( xmlReader->name()=="TriggeringTimeSecs" )
                raspcamSettings->TriggeringTimeSecs = xmlReader->readElementText().toInt(0);
            if( xmlReader->name()=="CameraMp" )
                raspcamSettings->CameraMp = xmlReader->readElementText().toInt(0);
            if( xmlReader->name()=="Flipped" )
                raspcamSettings->Flipped = xmlReader->readElementText().toInt(0);
        }
    }
    if(xmlReader->hasError()) {
        funcShowMsg(filePath+" parse Error",xmlReader->errorString());
        return false;
    }
    xmlReader->clear();
    xmlFile->close();

    return true;

}

int connectSocket( structCamSelected *camSelected ){

    //Check if Camera is On
    //QString IP((const char*)camSelected->IP);
    //QTcpSocket *tcpSocket = new QTcpSocket(Q_NULLPTR);
    //tcpSocket->connectToHost(IP,camSelected->tcpPort);
    //if(!tcpSocket->waitForConnected(3000))
    //{
    //     return -1;
    //}


    //Prepare command message
    int sockfd;
    //unsigned char bufferRead[sizeof(frameStruct)];
    struct sockaddr_in serv_addr;
    struct hostent *server;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    qDebug() << "Comm IP: " << QString((char*)camSelected->IP);
    if (sockfd < 0){
        qDebug() << "connectSocket: opening socket";
        return -1;
    }
    //server = gethostbyname( ui->tableLstCams->item(tmpRow,1)->text().toStdString().c_str() );
    server = gethostbyname( (char*)camSelected->IP );
    if (server == NULL) {
        qDebug() << "connectSocket: no such host";
        return -1;
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
        (char *)&serv_addr.sin_addr.s_addr,
        server->h_length);
    serv_addr.sin_port = htons(camSelected->tcpPort);
    if (::connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
        qDebug() << "connectSocket: connecting";
        return -1;
    }
    return sockfd;
}

void funcObtSettings( structSettings *lstSettings ){

    QFile *xmlFile = new QFile(_PATH_STARTING_SETTINGS);
    if (!xmlFile->open(QIODevice::ReadOnly | QIODevice::Text)) {
        funcShowMsg("ERROR","Couldn't open _PATH_STARTING_SETTINGS to load settings for download");
    }
    QXmlStreamReader *xmlReader = new QXmlStreamReader(xmlFile);


    //Parse the XML until we reach end of it
    while(!xmlReader->atEnd() && !xmlReader->hasError()) {
        // Read next element
        QXmlStreamReader::TokenType token = xmlReader->readNext();
        //If token is just StartDocument - go to next
        if(token == QXmlStreamReader::StartDocument) {
                continue;
        }
        //If token is StartElement - read it
        if(token == QXmlStreamReader::StartElement) {            
            if( xmlReader->name()=="version" ){
                lstSettings->version = xmlReader->readElementText().toFloat();
            }
            if( xmlReader->name()=="tcpPort" ){
                lstSettings->tcpPort = xmlReader->readElementText().toInt(0);
            }
        }
    }
    if(xmlReader->hasError()) {
        funcShowMsg("_PATH_STARTING_SETTINGS Parse Error",xmlReader->errorString());
    }
    xmlReader->clear();
    xmlFile->close();
}

void funcShowMsg(QString title, QString msg){
    QMessageBox yesNoMsgBox;
    yesNoMsgBox.setWindowTitle(title);
    yesNoMsgBox.setText(msg);
    yesNoMsgBox.setDefaultButton(QMessageBox::Ok);
    yesNoMsgBox.exec();
}

void funcShowMsgERROR(QString msg){
    QMessageBox yesNoMsgBox;
    yesNoMsgBox.setWindowTitle("ERROR");
    yesNoMsgBox.setText(msg);
    yesNoMsgBox.setDefaultButton(QMessageBox::Ok);
    yesNoMsgBox.exec();
}

int funcShowSelDir(QString startedPath, QString* dirSelected)
{
    *dirSelected = QFileDialog::getExistingDirectory(
                                                        NULL,
                                                        "Open Directory",
                                                        startedPath,
                                                        QFileDialog::ShowDirsOnly
                                                        | QFileDialog::DontResolveSymlinks);
    if( dirSelected->isEmpty() || dirSelected->isNull() )
    {
        dirSelected->clear();
        return _FAILURE;
    }
    dirSelected->append("/");
    //dirSelected = dirSelected;
    return _OK;
}

QString funcShowSelDir(QString path)
{
    QString dir = QFileDialog::getExistingDirectory(NULL, "Open Directory",
                                                 path,
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);
    return dir;
}

int func_getFilenameFromUser(QString* fileName, QWidget* parent)
{
    //
    //Read the filename
    //
    QString lastPath = readFileParam(_PATH_LAST_IMG_SAVED);
    if( lastPath.isEmpty() )//First time using this parameter
    {
        lastPath = "./snapshots/";
    }
    *fileName = QFileDialog::getSaveFileName(
                                                parent,
                                                "Save Snapshot as...",
                                                lastPath,
                                                "Documents (*.png)"
                                            );
    if( fileName->isEmpty() )
    {
        qDebug() << "Filename not typed";
        return _ERROR;
    }
    else
    {
        lastPath = funcRemoveFileNameFromPath(*fileName);
        saveFile(_PATH_LAST_IMG_SAVED,lastPath);
    }

    //
    //Validate filename
    //
    *fileName = funcRemoveImageExtension(*fileName);

    return _OK;
}

QString funcRemoveImageExtension( QString imgName )
{
    imgName.replace(".png","");
    imgName.replace(".PNG","");

    imgName.replace(".jpg","");
    imgName.replace(".JPG","");

    imgName.replace(".jpeg","");
    imgName.replace(".JPEG","");

    imgName.replace(".gif","");
    imgName.replace(".GIF","");

    imgName.replace(".rgb888","");
    imgName.replace(".RGB888","");

    imgName.replace(".bmp","");
    imgName.replace(".BMP","");

    imgName.append( _FRAME_EXTENSION );

    return imgName;

}

int func_getMultiImages(QStringList* fileNames, QWidget* parent)
{
    //Get last path
    QString lastPath = readFileParam(_PATH_LAST_IMG_SAVED);
    if( lastPath.isEmpty() )lastPath = "./snapshots/";

    //Get lst Images
    QFileDialog dialog(parent);
    dialog.setDirectory(lastPath);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter("Images (*.png *.RGB888)");
    dialog.setFileMode(QFileDialog::ExistingFiles);
    int dialogCode;
    dialogCode = dialog.exec();
    if( dialogCode )
        *fileNames = dialog.selectedFiles();
    if( dialogCode == QDialog::Accepted )
        return _OK;

    //Default
    return _FAILURE;
}

void funcShowFileError(int error, QString fileName){
    switch(error){
        case -1:
            funcShowMsg("ERROR","Empty file: " + fileName);
            break;
        case -2:
            funcShowMsg("ERROR","Unknow error in file: " + fileName);
            break;
        case -3:
            funcShowMsg("ERROR","File does not exists: " + fileName);
            break;
    }
}

void funcPrintFirst(int n, int max, char *buffer){
  QString Items;
  int i;
  Items.append("First: ");
  for(i=0;i<n;i++){
      Items.append(" "+ QString::number((int)((unsigned char)buffer[i])) +" ");
  }
  Items.append("\nLast: ");
  for(i=max-1;i>=max-n;i--){
    Items.append(" "+ QString::number((int)((unsigned char)buffer[i])) +" ");
  }
  Items.append("\n");
  funcShowMsg("First-Last",Items);
}

/*
void funcObtainIP(char* host){
    FILE *f;
    char line[100] , *p , *c;
    f = fopen("/proc/net/route" , "r");
    while(fgets(line , 100 , f)){
        p = strtok(line , " \t");
        c = strtok(NULL , " \t");
        if(p!=NULL && c!=NULL){
            if(strcmp(c , "00000000") == 0){
                //printf("Default interface is : %s \n" , p);
                break;
            }
        }
    }

    //which family do we require , AF_INET or AF_INET6
    int fm = AF_INET;
    struct ifaddrs *ifaddr, *ifa;
    int family , s;
    if (getifaddrs(&ifaddr) == -1){
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    //Walk through linked list, maintaining head pointer so we can free list later
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next){
        if (ifa->ifa_addr == NULL){
            continue;
        }
        family = ifa->ifa_addr->sa_family;
        if(strcmp( ifa->ifa_name , p) == 0){
            if (family == fm) {
                s = getnameinfo( ifa->ifa_addr, (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6) , host , NI_MAXHOST , NULL , 0 , NI_NUMERICHOST);
                if (s != 0) {
                    printf("getnameinfo() failed: %s\n", gai_strerror(s));
                    exit(EXIT_FAILURE);
                }
            }
            printf("\n");
        }
    }
    freeifaddrs(ifaddr);
}*/

bool funcIsIP( std::string ipCandidate ){
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


void QtDelay( unsigned int ms ){
    QTime dieTime= QTime::currentTime().addMSecs(ms);
    while (QTime::currentTime() < dieTime){
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
}

int xyToIndex( int x, int y, int w)
{
    return ((y-1)*w) + x;
}


int funcShowMsgYesNo( QString title, QString msg ){
    //int integerValue = 10;
    QMessageBox yesNoMsgBox;
    yesNoMsgBox.setWindowTitle(title);
    yesNoMsgBox.setText(msg);
    //yesNoMsgBox.setInformativeText(QString(info).append(QVariant(integerValue).toString()));
    //yesNoMsgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    yesNoMsgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    switch (yesNoMsgBox.exec()){
        case QMessageBox::Yes: {
            return 1;
            break;
        }
        case QMessageBox::No: {
            return 0;
            break;
        }
    }
    return -1;
}


void funcRGB2XYZ(colSpaceXYZ *spaceXYZ, float Red, float Green, float Blue){
    memset(spaceXYZ,'\0',sizeof(colSpaceXYZ));
    //http://docs.opencv.org/2.4/modules/imgproc/doc/miscellaneous_transformations.html
    //qDebug() << "R: " << QString::number(Red);
    //qDebug() << "G: " << QString::number(Green);
    //qDebug() << "B: " << QString::number(Blue);
    spaceXYZ->X = (0.412453*Red) + (0.35758*Green) + (0.180423*Blue);
    spaceXYZ->Y = (0.212671*Red) + (0.71516*Green) + (0.072169*Blue);
    spaceXYZ->Z = (0.019334*Red) + (0.119193*Green) + (0.950227*Blue);
    float acum  = spaceXYZ->X + spaceXYZ->Y + spaceXYZ->Z;
    spaceXYZ->x = spaceXYZ->X / acum;
    spaceXYZ->y = spaceXYZ->Y / acum;
    spaceXYZ->z = spaceXYZ->Z / acum;
}


linearRegresion *funcCalcLinReg( float *X ){
    linearRegresion *linReg = (linearRegresion*)malloc(sizeof(linearRegresion));
    float mX, mY, aux1, aux2;
    float Y[]   = { 435.7, 546.1, 611.3 };
    //Calculate linear regression
    mX          = (X[0]+X[1]+X[2]) / 3.0;
    mY          = (Y[0]+Y[1]+Y[2]) / 3.0;
    aux1        = ((X[0]-mX)*(Y[0]-mY)) + ((X[1]-mX)*(Y[1]-mY)) + ((X[1]-mX)*(Y[1]-mY));
    aux2        = ((X[0]-mX)*(X[0]-mX)) + ((X[1]-mX)*(X[1]-mX)) + ((X[2]-mX)*(X[2]-mX));
    linReg->b   = aux1 / aux2;
    linReg->a   = mY-(linReg->b*mX);
    return linReg;
}

linearRegresion funcLinearRegression( double *X, double *Y, int numItems ){

    if(false)
    {
        for(int i=0; i<numItems; i++)
        {
            printf("%lf, %lf\n",X[i],Y[i]);
        }
    }


    linearRegresion linReg;
    double mX=0.0, mY=0.0, aux1=0.0, aux2=0.0;
    int i;
    //Mean
    for(i=0;i<numItems;i++)
    {
        mX += X[i];
        mY += Y[i];
    }
    mX /= (double)numItems;
    mY /= (double)numItems;
    //funcShowMsg("mX,mY",QString::number(mX)+", "+QString::number(mY));
    //
    for(i=0;i<numItems;i++)
    {
        aux1 += (X[i]-mX)*(Y[i]-mY);
        aux2 += (X[i]-mX)*(X[i]-mX);
    }    
    linReg.b   = aux1 / aux2;
    linReg.a   = mY-(linReg.b*mX);

    //printf("linReg->b: %lf \n",linReg->b);
    //printf("aux1: %lf \n",aux1);
    //printf("aux2: %lf \n",aux2);
    //printf("mX: %lf \n",mX);
    //printf("mY: %lf \n",mY);

    //
    return linReg;
}


bool saveBinFile(unsigned long datasize, unsigned char *dataPtr, QString directory){
    QFile DummyFile(directory);
    if(DummyFile.open(QIODevice::WriteOnly)) {
        qint64 bytesWritten = DummyFile.write(reinterpret_cast<const char*>(dataPtr), datasize);
        if (bytesWritten < (qint64)datasize) {
            return false;
        }
        DummyFile.close();
    }
    return true;
}

/*
int saveBinFile_From_u_int8_T( std::string fileName, uint8_t *data, size_t len)
{
    std::ofstream fp;
    fp.open( fileName, std::ios::out | std::ios::binary );
    fp.write((char*)data, len);
    fp.close();
    return 1;
}*/

int fileExists( QString fileName )
{
    //Receive a filename and return
    //1: file exists
    //0: file does not exist
    //-1: fileName empty
    //-2: Unknow error
    if( fileName.isEmpty() )return -1;

    QFile tmpFile( fileName );
    if( tmpFile.exists() )
        return 1;
    else
        return 0;

    return -2;
}

int funcExecuteCommand( QString command )
{
    //Execute raspistill
    FILE* pipe;
    pipe = popen(command.toStdString().c_str(), "r");
    pclose(pipe);


    return -1;
}


QString funcExecuteCommandAnswer( char* command )
{
    //Execute command
    QString result;
    result = "";//idMsg to send
    FILE* pipe;
    pipe = popen(command, "r");
    char bufferComm[100];
    try
    {
      while (!feof(pipe))
      {
        if (fgets(bufferComm, frameBodyLen, pipe) != NULL)
        {
          result.append( bufferComm );
        }
      }
    }
    catch (...)
    {
      pclose(pipe);
      throw;
    }
    pclose(pipe);
    return result;
}

/*
void funcSourcePixToDiffPix(strDiffPix *diffPix, lstDoubleAxisCalibration *calSett ){
    diffPix->rightY = (float)calSett->rightLinRegA  + ( (float)calSett->rightLinRegB    * (float)diffPix->x );
    diffPix->upY    = (float)calSett->upLinRegA     + ( (float)calSett->upLinRegB       * (float)diffPix->x );
    diffPix->leftY  = (float)calSett->leftLinRegA   + ( (float)calSett->leftLinRegB     * (float)diffPix->x );
    diffPix->downY  = (float)calSett->downLinRegA   + ( (float)calSett->downLinRegB     * (float)diffPix->x );

    qDebug() << "inside: funcSourcePixToDiffPix";
    qDebug() << "x: " << diffPix->x;

    qDebug() << "calSett->rightLinRegA: " << calSett->rightLinRegA;
    qDebug() << "calSett->rightLinRegB: " << calSett->rightLinRegB;
    qDebug() << "calSett->rightY: " << diffPix->rightY;

}
*/


double funcDet2x2(double **M){
    return (M[0][0] * M[1][1]) - (M[1][0]*M[0][1]);
}

void funcOpenFolder(QString path)
{
    QDesktopServices::openUrl(QUrl(path));
}

int funcAccountFilesInDir(QString Dir)
{
    int numFiles = 0;
    QDir dir(Dir);
    if ( dir.exists() )
    {
        Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden | QDir::Files, QDir::DirsFirst))
        {
            numFiles++;
        }
    }
    return numFiles;
}

QList<QFileInfo> funcListFilesInDir(QString Dir)
{
    QList<QFileInfo> lstFiles;
    QDir dir(Dir);
    if ( dir.exists() )
    {
        Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden | QDir::Files, QDir::DirsFirst))
        {
            lstFiles.append(info);
        }
    }
    return lstFiles;
}

QList<QFileInfo> funcListFilesInDir(QString Dir, QString Suffix)
{
    Suffix = Suffix.replace(".","");

    //Only add files with suffix provided
    QList<QFileInfo> lstFiles;
    QDir dir(Dir);
    if ( dir.exists() )
    {
        Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden | QDir::Files, QDir::Time | QDir::Name | QDir::Reversed))
        {
            if( info.completeSuffix() == Suffix )
            {
                lstFiles.append(info);
            }
        }
    }
    return lstFiles;
}

QList<QFileInfo> funcFilterFilelist(QList<QFileInfo> lstFiles, QString suffix)
{
    int i;
    QList<QFileInfo> lstResult;
    for( i=0; i<lstFiles.size(); i++ )
    {
        if( lstFiles.at(i).completeSuffix() == suffix )
        {
            lstResult.append(lstFiles.at(i));
        }
    }
    return lstResult;
}

void funcClearDirFolder(QString path)
{
    QDir dir(path);
    if (dir.exists())
    {
        Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst))
        {
            QFile::remove(info.absoluteFilePath());
        }
    }
    else
    {
        dir.mkdir(path);
    }
}

int funcPrintRectangle(QString title, squareAperture* rectangle)
{
    qDebug() << "";
    qDebug() << title;
    qDebug() << "W: " << rectangle->canvasW;
    qDebug() << "H: " << rectangle->canvasH;
    qDebug() << "x: " << rectangle->rectX;
    qDebug() << "y: " << rectangle->rectY;
    qDebug() << "w: " << rectangle->rectW;
    qDebug() << "h: " << rectangle->rectH;
    return 1;
}

void funcNDVI( QImage* imgToNDVI, double lowerBound, int brilliant )
{
    //......................................
    // Get Infrared Weight
    //......................................
    QString infraredWeight;
    double infraredWeightRatio;
    infraredWeight = readFileParam( _PATH_NDVI_INFRARED_WEIGHT );
    infraredWeightRatio = infraredWeight.toDouble(0);
    if( infraredWeightRatio <= 0.0 )
    {
        funcShowMsg("FAIL","infraredWeightRatio WRONG!, setted to 1.0");
        infraredWeightRatio = 1.0;
    }

    //......................................
    // Get Minimum Value
    //......................................
    QString stringMinimumValue;
    double minimumValue;
    stringMinimumValue = readFileParam( _PATH_NDVI_MIN_VALUE );
    minimumValue = stringMinimumValue.toDouble(0);
    if( minimumValue < 1.0 || minimumValue > 255.0 )
    {
        funcShowMsg("FAIL","minimumValue WRONG!, setted to 1.0");
        minimumValue = 1.0;
    }

    //......................................
    // Validate lower bound
    //......................................
    if( lowerBound < -1.0 || lowerBound > 1.0 )
    {
        lowerBound = 0.0;
        funcShowMsg("Alert!","Lower bound fixed to 0.0");
    }

    //......................................
    // Apply Classic NDVI
    //......................................
    int x, y;
    double NDVI, maxNDVI, minNDVI, range;
    QRgb tmpPixel;
    maxNDVI = 0.0;
    minNDVI = 1.0;
    range   = 1.0 - lowerBound;
    double infraredSensed;
    double redSensed;
    for( x=0; x<imgToNDVI->width(); x++ )
    {
        for( y=0; y<imgToNDVI->height(); y++ )
        {
            //Calculate NDVI
            tmpPixel        = imgToNDVI->pixel(x,y);
            infraredSensed  = (double)qBlue(tmpPixel) * infraredWeightRatio;
            redSensed       = (double)qRed(tmpPixel);

            //Validate measuremente
            if( infraredSensed > minimumValue && redSensed > minimumValue )
                NDVI        = (infraredSensed-redSensed)/(infraredSensed+redSensed);
            else
                NDVI        = -1.0;


            //Uses threshold
            NDVI    = (NDVI>lowerBound)?NDVI:0.0;

            /*
            //Draw pixel
            if( NDVI >= -1.0 && NDVI < -0.2 )
                imgToNDVI->setPixel(x,y,qRgb(0,0,0));       //Negro
            if( NDVI >= -0.2 && NDVI < -0.1 )
                imgToNDVI->setPixel(x,y,qRgb(255,0,0));     //Rojo brillante
            if( NDVI >= -0.1 && NDVI < 0.0 )
                imgToNDVI->setPixel(x,y,qRgb(180,0,0));     //Rojo Sangre
            if( NDVI >= 0.0 && NDVI < 0.1 )
                imgToNDVI->setPixel(x,y,qRgb(90,0,0));      //Rojo Opaco
            if( NDVI >= 0.1 && NDVI < 0.2 )
                imgToNDVI->setPixel(x,y,qRgb(255,255,0));   //Amarillo Brillante
            if( NDVI >= 0.2 && NDVI < 0.3 )
                imgToNDVI->setPixel(x,y,qRgb(255,150,0));   //Amarillo Ocre
            if( NDVI >= 0.3 && NDVI < 0.4 )
                imgToNDVI->setPixel(x,y,qRgb(255,80,0));    //Verde Oliva
            if( NDVI >= 0.4 && NDVI < 0.5 )
                imgToNDVI->setPixel(x,y,qRgb(0,255,255));   //Azul Brillante
            if( NDVI >= 0.5 && NDVI < 0.6 )
                imgToNDVI->setPixel(x,y,qRgb(0,125,125));   //Azul Opaco
            if( NDVI >= 0.6 && NDVI < 0.7 )
                imgToNDVI->setPixel(x,y,qRgb(0,80,80));     //Azul obscuro
            if( NDVI >= 0.7 && NDVI < 0.8 )
                imgToNDVI->setPixel(x,y,qRgb(60,255,0));    //Verde Brillante
            if( NDVI >= 0.8 && NDVI < 0.9 )
                imgToNDVI->setPixel(x,y,qRgb(30,140,0));    //Verde Opaco
            if( NDVI >= 0.9 && NDVI < 1.0 )
                imgToNDVI->setPixel(x,y,qRgb(12,60,0));     //Verde militar*/

            //Draw pixel
            if( NDVI >= -1.0 && NDVI < -0.33 )
                imgToNDVI->setPixel(x,y,qRgb(0,0,0));
            if( NDVI >= -0.33 && NDVI < -0.1 )
                imgToNDVI->setPixel(x,y,qRgb(180,0,0));
            if( NDVI >= -0.1 && NDVI < 0.0 )
                imgToNDVI->setPixel(x,y,qRgb(50,0,0));
            if( NDVI >= 0.0 && NDVI < 0.1 )
                imgToNDVI->setPixel(x,y,qRgb(0,50,0));
            if( NDVI >= 0.1 && NDVI < 0.2 )
                imgToNDVI->setPixel(x,y,qRgb(0,100,0));
            if( NDVI >= 0.2 && NDVI < 0.3 )
                imgToNDVI->setPixel(x,y,qRgb(0,150,0));
            if( NDVI >= 0.3 && NDVI < 0.4 )
                imgToNDVI->setPixel(x,y,qRgb(0,200,0));
            if( NDVI >= 0.4 )
                imgToNDVI->setPixel(x,y,qRgb(0,255,0));




            //Save maximum
            //NDVI        = (NDVI>=lowerBound)?NDVI:0.0;
            maxNDVI     = (NDVI>maxNDVI)?NDVI:maxNDVI;
            minNDVI     = (NDVI<minNDVI)?NDVI:minNDVI;
        }
    }
    qDebug() << "maxNDVI: " << maxNDVI;
    qDebug() << "minNDVI: " << minNDVI;

    //......................................
    // Remark identified plat pixels
    //......................................
    if( brilliant )
    {
        int curve;
        curve = 255 - round( maxNDVI*255.0 );
        qDebug() << "curve: " << curve;
        if( curve > 5 )
        {
            for( x=0; x<imgToNDVI->width(); x++ )
            {
                for( y=0; y<imgToNDVI->height(); y++ )
                {
                    tmpPixel    = imgToNDVI->pixel(x,y);
                    NDVI        = qRed(tmpPixel) + curve;
                    if( qRed(tmpPixel) > 0 )
                        imgToNDVI->setPixel(x,y,qRgb(NDVI,0,0));
                }
            }
        }
    }
}


int funcReadAnalysePlot( structAnalysePlotSaved* structPlotSaved )
{

    QString tmpParameter = readFileParam( _PATH_SLIDE_FLUORESCENT );
    if( tmpParameter.isEmpty() )
    {
        qDebug() << _PATH_SLIDE_FLUORESCENT << " is empty";
        return _ERROR;
    }

    structPlotSaved->red        = tmpParameter.split(",").at(0).toInt(0);
    structPlotSaved->green      = tmpParameter.split(",").at(1).toInt(0);
    structPlotSaved->blue       = tmpParameter.split(",").at(2).toInt(0);
    structPlotSaved->canvasW    = tmpParameter.split(",").at(3).toInt(0);
    structPlotSaved->canvasH    = tmpParameter.split(",").at(4).toInt(0);
    structPlotSaved->originalW  = tmpParameter.split(",").at(5).toInt(0);
    structPlotSaved->originalH  = tmpParameter.split(",").at(6).toInt(0);

    return _OK;
}

void rotateQImage(QImage* tmpImg, int degree)
{
    QTransform transformation;
    transformation.rotate(degree);
    *tmpImg = tmpImg->transformed(transformation);
}

QRect screenResolution(QWidget* reference)
{
    return QApplication::desktop()->screenGeometry(QApplication::desktop()->screenNumber(reference));
}

int pixelMaxValue( QRgb pixel )
{
    int maxVal;
    maxVal = (qRed(pixel)>=qGreen(pixel))?qRed(pixel):qGreen(pixel);
    maxVal = (maxVal>=qBlue(pixel))?maxVal:qBlue(pixel);
    return maxVal;
}


QPoint imageSimilarity2D(QImage* img1, QImage* img2, float maxShift, bool horizontal)
{
    //maxShift is the percentage allowed to explore similarity

    //-----------------------------------------------------
    //Containers and Variables
    //-----------------------------------------------------
    QPoint shift2D;
    shift2D.setX(0);
    shift2D.setY(0);
    if( img1->width() < 1 || img1->height() < 1 )
        return shift2D;


    int k, n, pixelRange, numShifts, zeroError, kFix;
    pixelRange  = (horizontal==true)?round(img1->width()*maxShift):round(img1->height()*maxShift);
    numShifts   = (2*pixelRange)+1;
    n           = (horizontal==true)?img1->width():img1->height();

    //-----------------------------------------------------
    //Shifting
    //-----------------------------------------------------
    int kStart, kEnd;
    float minError, tmpError, shiftFix;
    kStart      = numShifts*(-1);
    kEnd        = numShifts;
    zeroError   = squareImageDifferenece( img1, img2, 0, horizontal );
    kFix        = (float)zeroError / (float)n;
    minError    = zeroError;
    for( k=kStart; k<kEnd; k++ )
    {
        shiftFix    = abs(k) * kFix;
        tmpError    = squareImageDifferenece( img1, img2, k, horizontal ) + shiftFix;
        if( tmpError < minError )
        {
            minError    = tmpError;
            shift2D.setY(k);
        }
    }

    return shift2D;
}

int squareImageDifferenece(QImage* img1, QImage* img2, int k, bool horizontal)
{
    int x, y, n, start, end, acum;
    QRgb pixelLeft, pixelRight;
    acum = 0;
    if(horizontal==true)
    {
        //...........................................
        //Horizontal
        //...........................................
        if( abs(k)>=img1->width() )
        {
            return 0;
        }
        n       = img1->width();
        end     = (k>=0)?n:n+k;
        start   = (k>=0)?k:0;
        for( x=start; x<end; x++ )
        {
            for( y=0; y<img1->height(); y++ )
            {
                pixelLeft   = img1->pixel( x, y );
                pixelRight  = img2->pixel( x-k, y );
                acum       += (qRed(pixelLeft)-qRed(pixelRight))*(qRed(pixelLeft)-qRed(pixelRight));
                acum       += (qGreen(pixelLeft)-qGreen(pixelRight))*(qGreen(pixelLeft)-qGreen(pixelRight));
                acum       += (qBlue(pixelLeft)-qBlue(pixelRight))*(qBlue(pixelLeft)-qBlue(pixelRight));
            }
        }
    }
    else
    {
        //...........................................
        //Vertical
        //...........................................
        if( abs(k)>=img1->height() )
        {
            return 0;
        }
        n       = img1->height();
        end     = (k>=0)?n:n+k;
        start   = (k>=0)?k:0;
        for( y=start; y<end; y++ )
        {
            for( x=0; x<img1->width(); x++ )
            {
                pixelLeft   = img1->pixel( x, y );
                pixelRight  = img2->pixel( x, y-k );
                acum       += (qRed(pixelLeft)-qRed(pixelRight))*(qRed(pixelLeft)-qRed(pixelRight));
                acum       += (qGreen(pixelLeft)-qGreen(pixelRight))*(qGreen(pixelLeft)-qGreen(pixelRight));
                acum       += (qBlue(pixelLeft)-qBlue(pixelRight))*(qBlue(pixelLeft)-qBlue(pixelRight));
            }
        }
    }
    return acum;
}

int vectorSimilarity(int* v1, int* v2, int n, float lang)
{
    float* c    = vectorCrossCorrelation(v1,v2,n,lang);
    return c[0]-n;
}

int* vectorConvolution(int* v1, int* v2, int n)
{

    //[Comment]: Apply Convolution, v1=x(n) and v2 = h(n)
    //           v1 and v3 have same dimensions
    //           v3[0] contains the position with the bigger value
    //           v3[1-n] contains the convolution result

    int numElem = 2*n;
    //qDebug() << "numElem: " << numElem;

    int i, j, maxAcum, aux, v1Pos, v2Pos, cPos;
    maxAcum  = 0;
    cPos     = 1;

    int* v3 = (int*)malloc(numElem*sizeof(int));
    memset( v3, '\0', numElem*sizeof(int));

    //Outside-Left to the Overlapped
    for( i=0; i<n; i++ )
    {
        aux = 0;
        for( j=0; j<=i; j++ )
        {
            v1Pos   = i-j;      //->
            v2Pos   = j;    //<-
            aux    += v1[v1Pos] * v2[v2Pos];
            //qDebug() << v1[v1Pos] << " x " << v2[v2Pos] << " -> " << aux;
        }
        //qDebug() << "---";
        //qDebug() << "i: " << i << " val: " << aux;
        v3[cPos] = aux;
        if( aux>maxAcum )
        {
            maxAcum  = aux;
            v3[0]    = cPos;
        }
        cPos++;
    }

    /*
    for( i=0; i<numElem; i++ )
    {
        qDebug() << v3[i];
    }
    exit(0);*/


    //Overlapped to Outside-Right
    for( i=1; i<n; i++ )
    {
        aux = 0;
        for( j=0; j<n-i; j++ )
        {
            v1Pos   = i+j;      //->
            v2Pos   = n-1-j;        //<-
            aux    += v1[v1Pos] * v2[v2Pos];
            //qDebug() << v1[v1Pos] << " x " << v2[v2Pos] << " -> " << aux;
        }
        //qDebug() << "---";
        //qDebug() << "i: " << i << " val: " << aux;
        v3[cPos] = aux;
        if( aux>maxAcum )
        {
            maxAcum  = aux;
            v3[0]    = cPos;
        }
        cPos++;
    }

    /*
    for( i=0; i<numElem; i++ )
    {
        qDebug() << v3[i];
    }
    exit(0);*/

    return v3;
}


float* vectorCrossCorrelation(int* v1, int* v2, int n, float lang)
{
    int k;
    int numElem = (2*n)+1;
    float* v3 = (float*)malloc(numElem*sizeof(float));
    memset( v3, '\0', numElem*sizeof(float));
    //lang = (lang>0.5)?0.5:lang;

    //---------------------------------------------
    //Calculate Correlation Zero Shift
    //---------------------------------------------
    float zeroCorr;
    zeroCorr        = vectorCorrelation(v1,v1,n,0);
    int maxShift    = floor((float)n*lang);
    maxShift        = ( maxShift > n-1 )?n-1:maxShift;
    int minShift    = maxShift*(-1);
    float maxCorr   = 0.0;
    for( k=minShift; k<=maxShift; k++ )
    {
        v3[n+k] = (k!=0)?vectorCorrelation(v1,v2,n,k,zeroCorr):zeroCorr;
        if( v3[n+k] > maxCorr )
        {
            maxCorr = v3[n+k];
            v3[0]   = n+k;
        }
    }

    /*
    for( k=0; k<=((2*n)-1); k++ )
    {
        qDebug() << "k: " << k-n << " v3: " << v3[k];
    }
    exit(0);*/


    return v3;
}

float vectorSimpleCorrelation(int* v1, int* v2, int n, int k)
{
    if( k>=n || k<((-1)*n) )
        return 0.0;
    int vectorAux[n];
    memset( vectorAux, '\0', (sizeof(int)*n) );

    //--------------------------------------------
    //Shift vectorAux=v2 if Required
    //--------------------------------------------
    int i, acum, begin, end;
    if( k>0  ){memcpy(vectorAux,&v2[k],(sizeof(int)*(n-k)));begin=0;end=n-k;}
    if( k<0  ){k=abs(k);memcpy(&vectorAux[k],v2,(sizeof(int)*(n-k)));begin=k;end=n;}
    if( k==0 ){memcpy( vectorAux, v2, (sizeof(int)*n) );begin=0;end=n;}

    //qDebug() << "begin: " << begin;
    //qDebug() << "end: " << end;

    //--------------------------------------------
    //Multiply v1 and v2=vectorAux
    //--------------------------------------------
    acum = 0;
    for( i=begin; i<end; i++ )
    {        
        acum += v1[i] * vectorAux[i];
        //qDebug() << v1[i] << " x " << vectorAux[i] << ": " << acum;
    }

    //--------------------------------------------
    //Calculate the Result
    //--------------------------------------------
    return (1.0/(float)n) * (float)acum;
}


float vectorCorrelation(int* v1, int* v2, int n, int k)
{
    //Calculate Raw Correlation
    float zeroCorr, corr, error;
    corr        = vectorSimpleCorrelation(v1,v2,n,k);

    //Fix Shipping Errro
    k = abs(k);
    zeroCorr    = vectorSimpleCorrelation(v1,v2,n,0);
    error       = 0;
    if( k != 0 )
    {
        error = ((float)k/(float)n)*zeroCorr;
        //qDebug() << "zeroCorr: " << zeroCorr << " corr: " << corr << " error: " << error;
        corr += error;
        //qDebug() << "corr final: " << corr;
    }
    //exit(0);

    return corr;
}

float vectorCorrelation(int* v1, int* v2, int n, int k, float zeroCorr)
{
    //Calculate Raw Correlation
    float corr, error;
    corr        = vectorSimpleCorrelation(v1,v2,n,k);

    //Fix Shipping Error
    k = abs(k);
    error       = 0;
    if( k != 0 )
    {
        error = ((float)k/(float)n)*zeroCorr;
        corr += error;
    }

    return corr;
}


int calcSlideExtraW(structSlideHypCube* slideSett)
{
    return floor((float)slideSett->width * slideSett->extraW);
}

QString funcGetParam(QString field)
{
    bool ok;
    return QInputDialog::getText(NULL, "Input required...",
                                                        field+":", QLineEdit::Normal,
                                                        "", &ok);
}

void funcSetFileDB()
{
    QList<QString> lstFolders;
    lstFolders << "XML" << "SYNC" << "./XML/camPerfils/" << "settings" << "./settings/lastPaths/" << "./settings/NDVI/";
    for( int i=0; i<lstFolders.size(); i++ )
    {
        if( !QDir( lstFolders.at(i) ).exists() )
        {
            QDir().mkdir( lstFolders.at(i) );
            funcShowMsg("Creating",lstFolders.at(i));
        }
    }
}









#include "formsettings.h"
#include "ui_formsettings.h"

#include <__common.h>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>

structRaspcamSettings *raspcamSettings = (structRaspcamSettings*)malloc(sizeof(structRaspcamSettings));

//QWidget *settingsParent;

formSettings::formSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::formSettings)
{
    ui->setupUi(this);

    //Prepare Settings Options
    funcSetSettingsOptions();


    //Initialize camera parameters
    ui->txtCamParamXMLName->setText("raspcamSettings");
    if ( fileExists( _PATH_RASPICAM_SETTINGS ) )
    {
        funcGetRaspParamFromXML( raspcamSettings, _PATH_RASPICAM_SETTINGS );
        funcIniCamParam( raspcamSettings );
    }
    else
    {
        funcShowMsg_Timeout("ALERT: It is your first time...","Please, It is necessary to set and save settings.");
    }

    //Hide Parent
    //settingsParent = parent;
    //settingsParent->setVisible(false);



}

formSettings::~formSettings()
{
    delete ui;
}

void formSettings::on_pbCopyShutter_clicked()
{    
    //settingsParent->setVisible(true);
    this->close();
}

void formSettings::on_pbSaveRaspParam_clicked()
{
    if( ui->txtCamParamXMLName->text().isEmpty() ){
        funcShowMsg_Timeout("Lack","Type the scenario's name");
    }else{

        QString tmpName = ui->txtCamParamXMLName->text();
        tmpName.replace(".xml","");
        tmpName.replace(".XML","");
        //qDebug() << tmpName;

        bool saveFile = false;
        if( QFileInfo::exists( "./XML/camPerfils/" + tmpName + ".xml" ) ){
            if( funcShowMsgYesNo("Alert","Replace existent file?") ){
                QFile file( "./XML/camPerfils/" + tmpName + ".xml" );
                file.remove();
                saveFile = true;
            }
        }else{
            saveFile = true;
        }

        if( saveFile ){
            if( saveRaspCamSettings( tmpName ) ){
                funcShowMsg_Timeout("Success","File stored successfully");
            }else{
                funcShowMsg_Timeout("ERROR","Saving raspcamsettings");
            }
        }
    }
}


bool formSettings::saveRaspCamSettings( QString tmpName )
{

    //----------------------------------------------
    //Conditional variables
    //----------------------------------------------
    if( tmpName.isEmpty() )return false;

    tmpName = tmpName.replace(".xml","");
    tmpName = tmpName.replace(".XML","");

    //Resolution
    int tmpResInMp = -1;
    if( ui->radioRes5Mp->isChecked() )
    {
        tmpResInMp = 5;
    }
    else
    {
        tmpResInMp = 8;
    }

    //----------------------------------------------
    //Prepare file contain
    //----------------------------------------------
    QString newFileCon = "";
    QString flipped = (ui->cbFlipped->isChecked())?"1":"0";
    QString denoiseFlag = (ui->cbDenoise->isChecked())?"1":"0";
    QString colbalFlag = (ui->cbColorBalance->isChecked())?"1":"0";
    newFileCon.append("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
    newFileCon.append("<settings>\n");
    newFileCon.append("    <AWB>"+ ui->cbAWB->currentText() +"</AWB>\n");
    newFileCon.append("    <Exposure>"+ ui->cbExposure->currentText() +"</Exposure>\n");
    newFileCon.append("    <Denoise>"+ denoiseFlag +"</Denoise>\n");
    newFileCon.append("    <ColorBalance>"+ colbalFlag +"</ColorBalance>\n");
    newFileCon.append("    <TriggeringTimeSecs>"+ QString::number( ui->slideTriggerTime->value() ) +"</TriggeringTimeSecs>\n");
    newFileCon.append("    <ShutterSpeedMs>"+ QString::number( ui->spinBoxShuterSpeed->value() ) +"</ShutterSpeedMs>\n");
    newFileCon.append("    <SquareShutterSpeedMs>"+ QString::number( ui->spinBoxSquareShuterSpeed->value() ) +"</SquareShutterSpeedMs>\n");
    newFileCon.append("    <TimelapseDurationSecs>"+ QString::number( ui->spinBoxTimelapseDuration->value() ) +"</TimelapseDurationSecs>\n");
    newFileCon.append("    <TimelapseInterval_ms>"+ QString::number( ui->spinBoxTimelapse->value() ) +"</TimelapseInterval_ms>\n");
    newFileCon.append("    <VideoDurationSecs>"+ QString::number( ui->spinBoxVideoDuration->value() ) +"</VideoDurationSecs>\n");
    newFileCon.append("    <ISO>"+ QString::number( ui->slideISO->value() ) +"</ISO>\n");
    newFileCon.append("    <CameraMp>"+ QString::number( tmpResInMp ) +"</CameraMp>\n");
    newFileCon.append("    <Flipped>"+ flipped +"</Flipped>\n");
    newFileCon.append("</settings>");

    //----------------------------------------------
    //Save
    //----------------------------------------------
    QFile newFile( "./XML/camPerfils/" + tmpName + ".xml" );
    if(newFile.exists())newFile.remove();
    if ( newFile.open(QIODevice::ReadWrite) ){
        QTextStream stream( &newFile );
        stream << newFileCon << endl;
        newFile.close();
    }else{
        return false;
    }
    return true;
}

void formSettings::on_pbObtPar_clicked()
{
    this->setVisible(false);


    QString filePath = QFileDialog::getOpenFileName(
                                                        this,
                                                        tr("Select XML..."),
                                                        "./XML/camPerfils/",
                                                        "(*.xml);;"
                                                     );
    if( !filePath.isEmpty() ){
        QStringList tmpList = filePath.split("/");
        tmpList = tmpList.at( tmpList.count()-1 ).split(".");
        ui->txtCamParamXMLName->setText( tmpList.at( 0 ) );

        funcGetRaspParamFromXML( raspcamSettings, filePath );
        funcIniCamParam( raspcamSettings );
    }

    this->setVisible(true);

}

void formSettings::funcIniCamParam( structRaspcamSettings *raspcamSettings )
{
    QList<QString> tmpList;

    //Set AWB: off,auto,sun,cloud,shade,tungsten,fluorescent,incandescent,flash,horizon
    tmpList<<"none"<<"off"<<"auto"<<"sun"<<"cloud"<<"shade"<<"tungsten"<<"fluorescent"<<"incandescent"<<"flash"<<"horizon";
    ui->cbAWB->clear();
    ui->cbAWB->addItems( tmpList );
    ui->cbAWB->setCurrentText((char*)raspcamSettings->AWB);
    tmpList.clear();

    //Set Exposure: off,auto,night,nightpreview,backlight,spotlight,sports,snow,beach,verylong,fixedfps,antishake,fireworks
    tmpList<<"none"<<"off"<<"auto"<<"night"<<"nightpreview"<<"backlight"<<"spotlight"<<"sports"<<"snow"<<"beach"<<"verylong"<<"fixedfps"<<"antishake"<<"fireworks";
    ui->cbExposure->clear();
    ui->cbExposure->addItems( tmpList );
    ui->cbExposure->setCurrentText((char*)raspcamSettings->Exposure);
    tmpList.clear();

    //Gray YUV420
    //if( raspcamSettings->Format == 1 )ui->rbFormat1->setChecked(true);
    //if( raspcamSettings->Format == 2 )ui->rbFormat2->setChecked(true);

    //Brightness
    //ui->slideBrightness->setValue( raspcamSettings->Brightness );
    //ui->labelBrightness->setText( "Brightness: " + QString::number(raspcamSettings->Brightness) );

    //Sharpness
    //ui->slideSharpness->setValue( raspcamSettings->Sharpness );
    //ui->labelSharpness->setText( "Sharpness: " + QString::number(raspcamSettings->Sharpness) );

    //Contrast
    //ui->slideContrast->setValue( raspcamSettings->Contrast );
    //ui->labelContrast->setText( "Contrast: " + QString::number(raspcamSettings->Contrast) );

    //Saturation
    //ui->slideSaturation->setValue( raspcamSettings->Saturation );
    //ui->labelSaturation->setText( "Saturation: " + QString::number(raspcamSettings->Saturation) );

    //DiffractionShuterSpeed
    ui->spinBoxShuterSpeed->setValue( raspcamSettings->ShutterSpeedMs );

    //SquareShuterSpeed
    ui->spinBoxSquareShuterSpeed->setValue(raspcamSettings->SquareShutterSpeedMs);

    //Video Duration
    ui->spinBoxVideoDuration->setValue( raspcamSettings->VideoDurationSecs );

    //Timelapse Duration
    ui->spinBoxTimelapseDuration->setValue( raspcamSettings->TimelapseDurationSecs );

    //Timelapse Interval
    ui->spinBoxTimelapse->setValue( raspcamSettings->TimelapseInterval_ms );

    //ISO
    ui->slideISO->setValue( raspcamSettings->ISO );
    ui->labelISO->setText( "ISO: " + QString::number(raspcamSettings->ISO) );

    //ExposureCompensation
    //ui->slideExpComp->setValue( raspcamSettings->ExposureCompensation );
    //ui->labelExposureComp->setText( "Exp. Comp.: " + QString::number(raspcamSettings->ExposureCompensation) );

    //RED
    //qDebug() << "Red: " << raspcamSettings->Red;
    //ui->slideRed->setValue( raspcamSettings->Red );
    //ui->labelRed->setText( "Red: " + QString::number(raspcamSettings->Red) );

    //GREEN
    //ui->slideGreen->setValue( raspcamSettings->Green );
    //ui->labelGreen->setText( "Green: " + QString::number(raspcamSettings->Green) );

    //PREVIEW
    //if( raspcamSettings->Preview )ui->cbPreview->setChecked(true);
    //else ui->cbPreview->setChecked(false);

    //TRIGGER TIME
    ui->slideTriggerTime->setValue(raspcamSettings->TriggeringTimeSecs);
    ui->labelTriggerTime->setText("Trigger time: " + QString::number(raspcamSettings->TriggeringTimeSecs)+" secs");

    //DENOISE EFX
    if( raspcamSettings->Denoise )ui->cbDenoise->setChecked(true);
    else ui->cbDenoise->setChecked(false);

    //FULL PHOTO
    //if( raspcamSettings->FullPhoto )ui->cbFullPhoto->setChecked(true);
    //else ui->cbFullPhoto->setChecked(false);

    //COLORBALANCE EFX
    if( raspcamSettings->ColorBalance )ui->cbColorBalance->setChecked(true);
    else ui->cbColorBalance->setChecked(false);

    //CAMERA RESOLUTION
    if( raspcamSettings->CameraMp == 5 )ui->radioRes5Mp->setChecked(true);
    else ui->radioRes8Mp->setChecked(true);

    //FLIPPED
    if( raspcamSettings->Flipped )ui->cbFlipped->setChecked(true);
    else ui->cbFlipped->setChecked(false);


}

void formSettings::funcSetSettingsOptions()
{

    QList<QString> tmpList;

    //Set AWB: off,auto,sun,cloud,shade,tungsten,fluorescent,incandescent,flash,horizon
    tmpList<<"none"<<"off"<<"auto"<<"sun"<<"cloud"<<"shade"<<"tungsten"<<"fluorescent"<<"incandescent"<<"flash"<<"horizon";
    ui->cbAWB->clear();
    ui->cbAWB->addItems( tmpList );
    tmpList.clear();

    //Set Exposure: off,auto,night,nightpreview,backlight,spotlight,sports,snow,beach,verylong,fixedfps,antishake,fireworks
    tmpList<<"none"<<"off"<<"auto"<<"night"<<"nightpreview"<<"backlight"<<"spotlight"<<"sports"<<"snow"<<"beach"<<"verylong"<<"fixedfps"<<"antishake"<<"fireworks";
    ui->cbExposure->clear();
    ui->cbExposure->addItems( tmpList );
    tmpList.clear();
}




void formSettings::funcShowMsg_Timeout(QString title, QString msg, int ms)
{
    QMessageBox *msgBox         = new QMessageBox(QMessageBox::Warning,title,msg,NULL);
    QTimer *msgBoxCloseTimer    = new QTimer(this);
    msgBoxCloseTimer->setInterval(ms);
    msgBoxCloseTimer->setSingleShot(true);
    connect(msgBoxCloseTimer, SIGNAL(timeout()), msgBox, SLOT(reject()));
    msgBoxCloseTimer->start();
    msgBox->exec();
}

void formSettings::funcShowMsg_Timeout(QString title, QString msg)
{
    QMessageBox *msgBox         = new QMessageBox(QMessageBox::Warning,title,msg,NULL);
    QTimer *msgBoxCloseTimer    = new QTimer(this);
    msgBoxCloseTimer->setInterval(2500);
    msgBoxCloseTimer->setSingleShot(true);
    connect(msgBoxCloseTimer, SIGNAL(timeout()), msgBox, SLOT(reject()));
    msgBoxCloseTimer->start();
    msgBox->exec();
}

void formSettings::funcShowMsgSUCCESS_Timeout(QString msg, int ms)
{
    QMessageBox *msgBox         = new QMessageBox(QMessageBox::Warning,"SUCCESS",msg,NULL);
    QTimer *msgBoxCloseTimer    = new QTimer(this);
    msgBoxCloseTimer->setInterval(ms);
    msgBoxCloseTimer->setSingleShot(true);
    connect(msgBoxCloseTimer, SIGNAL(timeout()), msgBox, SLOT(reject()));
    msgBoxCloseTimer->start();
    msgBox->exec();
}

void formSettings::funcShowMsgSUCCESS_Timeout(QString msg)
{
    QMessageBox *msgBox         = new QMessageBox(QMessageBox::Warning,"SUCCESS",msg,NULL);
    QTimer *msgBoxCloseTimer    = new QTimer(this);
    msgBoxCloseTimer->setInterval(2500);
    msgBoxCloseTimer->setSingleShot(true);
    connect(msgBoxCloseTimer, SIGNAL(timeout()), msgBox, SLOT(reject()));
    msgBoxCloseTimer->start();
    msgBox->exec();
}

void formSettings::funcShowMsgERROR_Timeout(QString msg, int ms)
{
    QMessageBox *msgBox         = new QMessageBox(QMessageBox::Warning,"ERROR",msg,NULL);
    QTimer *msgBoxCloseTimer    = new QTimer(this);
    msgBoxCloseTimer->setInterval(ms);
    msgBoxCloseTimer->setSingleShot(true);
    connect(msgBoxCloseTimer, SIGNAL(timeout()), msgBox, SLOT(reject()));
    msgBoxCloseTimer->start();
    msgBox->exec();
}

void formSettings::funcShowMsgERROR_Timeout(QString msg)
{
    QMessageBox *msgBox         = new QMessageBox(QMessageBox::Warning,"ERROR",msg,NULL);
    QTimer *msgBoxCloseTimer    = new QTimer(this);
    msgBoxCloseTimer->setInterval(2500);
    msgBoxCloseTimer->setSingleShot(true);
    connect(msgBoxCloseTimer, SIGNAL(timeout()), msgBox, SLOT(reject()));
    msgBoxCloseTimer->start();
    msgBox->exec();
}

void formSettings::on_slideISO_valueChanged(int value)
{
    ui->labelISO->setText( "ISO: " + QString::number(value) );
}

void formSettings::on_slideTriggerTime_valueChanged(int value)
{
    ui->labelTriggerTime->setText( "Trigger at: " + QString::number(value) + " secs" );
}

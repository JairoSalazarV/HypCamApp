#ifndef FORMSETTINGS_H
#define FORMSETTINGS_H

#include <QDialog>
#include <lstStructs.h>

namespace Ui {
class formSettings;
}

class formSettings : public QDialog
{
    Q_OBJECT

public:
    explicit formSettings(QWidget *parent = 0);
    ~formSettings();

private slots:
    void on_pbCopyShutter_clicked();

    void on_pbSaveRaspParam_clicked();

    bool saveRaspCamSettings( QString tmpName );

    void on_pbObtPar_clicked();

    void funcIniCamParam( structRaspcamSettings *raspcamSettings );

    void funcSetSettingsOptions();

    void funcShowMsg_Timeout(QString title, QString msg, int ms);
    void funcShowMsg_Timeout(QString title, QString msg);

    void funcShowMsgSUCCESS_Timeout(QString msg, int ms);
    void funcShowMsgSUCCESS_Timeout(QString msg);

    void funcShowMsgERROR_Timeout(QString msg, int ms);
    void funcShowMsgERROR_Timeout(QString msg);

    void on_slideISO_valueChanged(int value);

    void on_slideTriggerTime_valueChanged(int value);

private:
    Ui::formSettings *ui;
};

#endif // FORMSETTINGS_H

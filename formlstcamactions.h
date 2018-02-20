#ifndef FORMLSTCAMACTIONS_H
#define FORMLSTCAMACTIONS_H

#include <QDialog>

namespace Ui {
class formLstCamActions;
}

class formLstCamActions : public QDialog
{
    Q_OBJECT

public:
    int funcValidateFileDirNameDuplicated(QString remoteFile, QString localFile);

    QString funcGetSyncFolder();

    int funcGetRaspCamParameters();

    void funcMainCall_GetSnapshot();

    explicit formLstCamActions(QWidget *parent = 0);
    ~formLstCamActions();

private slots:
    void on_pbTestConnection_clicked();

    void on_pbTakeSnapshot_clicked();    

    void on_pbVideoRecording_clicked();

private:
    Ui::formLstCamActions *ui;
};

#endif // FORMLSTCAMACTIONS_H

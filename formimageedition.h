#ifndef FORMIMAGEEDITION_H
#define FORMIMAGEEDITION_H

#include <QDialog>

namespace Ui {
class formImageEdition;
}

class formImageEdition : public QDialog
{
    Q_OBJECT

public:
    explicit formImageEdition(QWidget *parent = 0);
    ~formImageEdition();

    void funcShowMsg_Timeout(QString title, QString msg, int ms);
    //void funcShowMsg_Timeout(QString title, QString msg);

    void funcShowMsgSUCCESS_Timeout(QString msg, int ms);
    void funcShowMsgSUCCESS_Timeout(QString msg);

    void funcShowMsgERROR_Timeout(QString msg, int ms);
    void funcShowMsgERROR_Timeout(QString msg);

    void funcUpdateShownImage();
    void funcUpdateShownImage(QImage imgToShow);

private slots:
    void on_pbSynLastSnapshot_clicked();

    //int obtainFile(std::string remoteFile, std::string localFile );

    //u_int8_t* funcQtReceiveFile( std::string fileNameRequested, int* fileLen );

    void on_pbNDVIAlgorithm_clicked();

private:
    Ui::formImageEdition *ui;
};

#endif // FORMIMAGEEDITION_H

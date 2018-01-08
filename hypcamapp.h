#ifndef HYPCAMAPP_H
#define HYPCAMAPP_H

#include <QMainWindow>
#include <lstStructs.h>

namespace Ui {
class HypCamApp;
}

class HypCamApp : public QMainWindow
{
    Q_OBJECT

public:
    explicit HypCamApp(QWidget *parent = 0);
    ~HypCamApp();

private slots:
    void on_pbCamConn_clicked();

    void on_pbShutdown_clicked();

    std::string funcRemoteTerminalCommand(
                                                std::string command,
                                                structCamSelected *camSelected,
                                                int trigeredTime,
                                                bool waitForAnswer,
                                                bool* ok
                                         );

    bool funcReceiveFile(
                            int sockfd,
                            unsigned int fileLen,
                            unsigned char *bufferRead,
                            unsigned char *tmpFile
                        );

private:
    Ui::HypCamApp *ui;
};

#endif // HYPCAMAPP_H

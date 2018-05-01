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

public slots:
    void resizeEvent(QResizeEvent *event);

private slots:
    void on_pbSettings_clicked();

    void on_pbSynSettings_clicked();

    void on_pbImgEdition_clicked();

    void on_pbAnalysis_clicked();

    void on_pbShutdown_clicked();

    void on_pbExit_clicked();

private:
    Ui::HypCamApp *ui;
};

#endif // HYPCAMAPP_H

#include "hypcamapp.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    HypCamApp w;
    w.show();    

    return a.exec();
}

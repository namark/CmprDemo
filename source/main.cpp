#include <QApplication>
#include "mainwindow.h"


void ProgressOutput(unsint p)
{
    Threading::completeness = p;
}


int main(int argc, char *argv[])
{
    QApplication a(argc,argv);
    MainWindow w;
    w.setPOFunc(ProgressOutput);
    w.show();

    return a.exec();
}


#include "gui/recivermainwindow.h"

#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    ReciverMainWindow w;
    w.show();

    return a.exec();
}

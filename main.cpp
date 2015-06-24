
/*
 * Reza Adhitya Saputra
 */

#include "mainwindow.h"
#include <QApplication>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setStyle(QStyleFactory::create("fusion"));
    //QPalette p;
    //p = qApp->palette();
    //p.setColor(QPalette::Button, QColor(200,200,200));
    //qApp->setPalette(p);

    MainWindow w;
    w.show();

    return a.exec();
}

#include "mainwindow.h"
#include <QApplication>

std::uniform_int_distribution<uint32_t> MainWindow::u(0, 1048575);
std::default_random_engine MainWindow::e;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}

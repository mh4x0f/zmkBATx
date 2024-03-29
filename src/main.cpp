#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QMenu>
#include <QMessageBox>
#include <QSystemTrayIcon>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("zmkBATx");
    QCoreApplication::setOrganizationDomain("zmkBATx");
    QCoreApplication::setApplicationName("zmkBATx");

    AppSettings *pref = AppSettings::getInstance();
    QFile file(":themes/assets/indigo.qss");
    file.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(file.readAll());
    a.setStyleSheet(styleSheet);

    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(nullptr, QApplication::applicationName(),QString("System tray unavailable"));
        return 1;
    }
    QApplication::setQuitOnLastWindowClosed(false);

    MainWindow w;
    w.setFixedSize(573,405);
    if (!pref->checkIfEmpty("current_device_name")){
        qDebug() << "Hide app to system tray";
        w.hide();
    }
    else w.show();

    return a.exec();
}
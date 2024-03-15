#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "appSettings.h"
#include "bleinterfacebluez.h"
#include <QMenu>
#include <QSystemTrayIcon>
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_bleScanButton_clicked();
    void on_connectButton_clicked();
    void on_disconnectButton_clicked();

public slots:
    void show_me();
    void quit_me();
    void show_aboutme();

private:
    Ui::MainWindow *ui;
    BLEInterfaceBluez *m_bleInterfaceBluez;
    QTimer mqTimerCheckingDeviceStatus;
    QSystemTrayIcon *trayIcon;
    QAction *Show_action;
    QAction *Preferences_action;
    QAction *Quit_action;
    AppSettings *pref;

    void dataReceived(QByteArray data);
    void dataReceivedBatteryInfo(const QHash<DeviceInfoBluez::BatteryType, int>  &dataBatteryInfo);
    void dataConnectedToDevice(const bool data);
};

#endif // MAINWINDOW_H

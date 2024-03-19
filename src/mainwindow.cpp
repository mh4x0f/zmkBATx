#include "mainwindow.h"
#include "about.h"
#include "ui_mainwindow.h"
#include <QStatusBar>
#include "appSettings.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    pref = AppSettings::getInstance();

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(":/icons/main.png"));
    trayIcon->setToolTip("zmk split keyboard");
    QMenu *changer_menu = new QMenu;
    Show_action = new QAction(tr("S&how"),this);
    Show_action->setIconVisibleInMenu(true);
    connect(Show_action, SIGNAL(triggered()), this, SLOT(show_me()));

    Quit_action = new QAction(tr("&Quit"), this);
    Quit_action->setIconVisibleInMenu(true);
    connect(Quit_action, &QAction::triggered,
            qApp, &QApplication::quit);

    connect(ui->actionQuit, &QAction::triggered,
            qApp, &QApplication::quit);

    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(show_aboutme()));

    changer_menu->addAction(Show_action);
    changer_menu->addAction(Quit_action);
    changer_menu->addSeparator();
    trayIcon->setContextMenu(changer_menu);
    trayIcon->show();

    m_bleInterfaceBluez = new BLEInterfaceBluez(this);
    connect(m_bleInterfaceBluez, &BLEInterfaceBluez::dataReceivedBatteryInfo,
            this, &MainWindow::dataReceivedBatteryInfo);
    connect(m_bleInterfaceBluez, &BLEInterfaceBluez::dataConnectedToDevice,
            this, &MainWindow::dataConnectedToDevice);
    connect(m_bleInterfaceBluez, &BLEInterfaceBluez::devicesNamesChanged,
            [this] (QStringList devices){
        ui->devicesComboBox->clear();
        ui->devicesComboBox->addItems(devices);
    });
    connect(m_bleInterfaceBluez, &BLEInterfaceBluez::currentDeviceChanged,
            [this] (int deviceID){
        ui->devicesComboBox->setCurrentIndex(deviceID);
        ui->progBarCentral->setValue(0);
        ui->progBarPeripheral->setValue(0);
    });
    connect(m_bleInterfaceBluez, &BLEInterfaceBluez::statusInfoChanged,
            [this](QString info, bool){
        this->statusBar()->showMessage(info);
    });

    connect(m_bleInterfaceBluez, &BLEInterfaceBluez::connectedChanged,
            [this](bool connected){
        if (!connected){
            ui->progBarCentral->setValue(0);
            ui->progBarPeripheral->setValue(0);
            this->statusBar()->showMessage("");
            this->trayIcon->setIcon(QIcon(":/icons/main.png"));
        }
    });
    m_bleInterfaceBluez->scanDevicesBluez();
    m_bleInterfaceBluez->loadConfigDevice();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::show_me(){
    this->show();
}

void MainWindow::show_aboutme(){
    About *about = new About();
    about->show();
}

void MainWindow::quit_me(){
    this->close();
}

void MainWindow::on_bleScanButton_clicked()
{
    m_bleInterfaceBluez->scanDevicesBluez();

}

void MainWindow::on_connectButton_clicked()
{
    m_bleInterfaceBluez->set_currentDevice(ui->devicesComboBox->currentIndex());
    m_bleInterfaceBluez->connectCurrentDevice();
}

void MainWindow::dataReceivedBatteryInfo(const QHash<DeviceInfoBluez::BatteryType, int>  &dataBatteryInfo){
    ui->progBarCentral->setValue(dataBatteryInfo[DeviceInfoBluez::BatteryType::Central]);
    ui->progBarPeripheral->setValue(dataBatteryInfo[DeviceInfoBluez::BatteryType::Peripheral]);
    int battery_level = dataBatteryInfo[DeviceInfoBluez::BatteryType::Central];
    if (battery_level <= 75 && battery_level >= 50)
        trayIcon->setIcon(QIcon(":/icons/m-battery.png"));
    else if (battery_level >= 75 && battery_level <= 100)
        trayIcon->setIcon(QIcon(":/icons/full-battery.png"));
    else if (battery_level <= 50 and battery_level >= 30)
        trayIcon->setIcon(QIcon(":/icons/half-battery.png"));
    else
        trayIcon->setIcon(QIcon(":/icons/low-battery.png"));
    trayIcon->setToolTip(
        QString("%1\nCentral: %2%\nPeripheral: %3%")
            .arg(ui->devicesComboBox->currentText().split(QRegularExpression("\\s+"), Qt::SkipEmptyParts)[0])
            .arg(dataBatteryInfo[DeviceInfoBluez::BatteryType::Central])
            .arg(dataBatteryInfo[DeviceInfoBluez::BatteryType::Peripheral])
        );
}


void MainWindow::dataConnectedToDevice(bool status)
{
    if (status){
        ui->connectButton->setEnabled(false);
    }
    ui->disconnectButton->setEnabled(true);
    if (!pref->checkIfEmpty("current_device_name"))
        ui->devicesComboBox->setCurrentIndex(
            m_bleInterfaceBluez->getDevicesNames()
                .indexOf(pref->value("current_device_name"))
        );
}

void MainWindow::on_disconnectButton_clicked()
{
    m_bleInterfaceBluez->disconnectDevice();
    ui->connectButton->setEnabled(true);
    ui->disconnectButton->setEnabled(false);
}


#ifndef BLEINTERFACEBLUEZ_H
#define BLEINTERFACEBLUEZ_H

#include <QObject>
#include <QTimer>
#include <QSettings>
#include "appSettings.h"
#include "qbluetoothuuid.h"
#include "simplebluez/Bluez.h"


class DeviceInfoBluez: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString deviceName READ getName NOTIFY deviceChanged)
    Q_PROPERTY(QString deviceAddress READ getAddress NOTIFY deviceChanged)
public:
    DeviceInfoBluez(const std::shared_ptr<SimpleBluez::Device>  &device);
    void setDevice(const  std::shared_ptr<SimpleBluez::Device> &device);
    QString getName() const { return QString::fromStdString(m_device->name()); }
    QString getAddress() const;
    std::shared_ptr<SimpleBluez::Device> getDevice() const;
    enum BatteryType {
        Central,
        Peripheral
    };
    Q_ENUM(BatteryType)

signals:
    void deviceChanged();

private:
    std::shared_ptr<SimpleBluez::Device> m_device;
};

class BLEInterfaceBluez : public QObject
{
    Q_OBJECT

public:
    explicit BLEInterfaceBluez(QObject *parent = 0);
    ~BLEInterfaceBluez();

    void connectCurrentDevice();
    void loadConfigDevice();
    void disconnectDevice();
    Q_INVOKABLE void scanDevicesBluez();

    bool isConnected() const
    {
        return mConnectedDevice;
    }

    QStringList getDevicesNames(){
        return mDevicesNames;
    }

public slots:
    void set_currentDevice(int currentDevice) {
        if (mCurrentDevice == currentDevice)
            return;
        emit currentDeviceChanged(currentDevice);
        mCurrentDevice = currentDevice;
    }

signals:
    void statusInfoChanged(QString info, bool Status);
    void dataReceivedBatteryInfo(const QHash<DeviceInfoBluez::BatteryType, int>  &dataBatteryInfo);
    void connectedChanged(bool connected);
    void dataConnectedToDevice(bool status);
    void devicesNamesChanged(QStringList devices);
    void currentDeviceChanged(int currentDeviceID);

private slots:
    void checkDevicePiredUpdate();

private:
    QTimer mqTimerCheckingDeviceStatus;
    AppSettings *pref;
    void update_connected(bool connected){
        if(connected != mConnectedDevice){
            mConnectedDevice = connected;
            emit connectedChanged(connected);
        }
    }

    void setConfigCurrentDevice();
    void addDevices(std::vector<std::shared_ptr<SimpleBluez::Device>> devices);
    void readCharacteristicByService(const std::shared_ptr<SimpleBluez::Service> &service);
    void readCharacteristicFromBatteryServices();

    std::vector<std::shared_ptr<SimpleBluez::Service>> findServicesByUuid(const QBluetoothUuid &qBluetoothUuid);

    QList<DeviceInfoBluez*> m_devices;
    std::vector<std::pair<std::shared_ptr<SimpleBluez::Service>, std::shared_ptr<SimpleBluez::Characteristic>>>
                            m_servicesList;
    QList<QBluetoothUuid> m_servicesUuid;

    SimpleBluez::Bluez m_bluez;
    std::vector<std::shared_ptr<SimpleBluez::Device>> m_peripherals;
    std::vector<std::shared_ptr<SimpleBluez::Adapter>> m_adapters;
    std::shared_ptr<SimpleBluez::Adapter> m_adapter_defualt;
    QHash<DeviceInfoBluez::BatteryType, int> hDeviceBatteryInfo;


    bool mConnectedDevice;
    QStringList mDevicesNames;
    int mCurrentDevice;
    QStringList mServices;
};



#endif // BLEINTERFACEBLUEZ_H

#include "bleinterfacebluez.h"
#include "appSettings.h"
#include <QDebug>
#include <QTimer>
#include <QSettings>
#include <src/thirdparty/simplebluez/Exceptions.h>


unsigned long hex2dec(std::string hex)
{
    // font: https://stackoverflow.com/questions/11031159/c-converting-hexadecimal-to-decimal
    unsigned long result = 0;
    for (int i=0; i<hex.length(); i++) {
        if (hex[i]>=48 && hex[i]<=57)
        {
            result += (hex[i]-48)*pow(16,hex.length()-i-1);
        } else if (hex[i]>=65 && hex[i]<=70) {
            result += (hex[i]-55)*pow(16,hex.length( )-i-1);
        } else if (hex[i]>=97 && hex[i]<=102) {
            result += (hex[i]-87)*pow(16,hex.length()-i-1);
        }
    }
    return result;
}


DeviceInfoBluez::DeviceInfoBluez(const std::shared_ptr<SimpleBluez::Device>  &device):
    QObject(), m_device(device)
{
}

std::shared_ptr<SimpleBluez::Device> DeviceInfoBluez::getDevice() const
{
    return m_device;
}

QString DeviceInfoBluez::getAddress() const
{
    return QString::fromStdString(m_device->address());
}

void DeviceInfoBluez::setDevice(const std::shared_ptr<SimpleBluez::Device> &device)
{
    m_device = device;
    emit deviceChanged();
}


BLEInterfaceBluez::BLEInterfaceBluez(QObject *parent) : QObject(parent) {
    pref = AppSettings::getInstance();
    m_bluez.init();
    m_bluez.run_async();
    m_adapters  = m_bluez.get_adapters();

    qWarning() <<  "Available adapters size: " << m_adapters.size() << Qt::endl;
    if (m_adapters.size() > 0)
        m_adapter_defualt = m_adapters[0];

    connect(&mqTimerCheckingDeviceStatus, &QTimer::timeout, this,
            &BLEInterfaceBluez::checkDevicePiredUpdate);
    mqTimerCheckingDeviceStatus.setInterval(pref->value("CONFIG_TIME_REPORT_INTERVAL").toInt());
    mqTimerCheckingDeviceStatus.start();
}


BLEInterfaceBluez::~BLEInterfaceBluez()
{
    qDeleteAll(m_devices);
    m_devices.clear();
}

void BLEInterfaceBluez::addDevices(std::vector<std::shared_ptr<SimpleBluez::Device>> devices)
{
    qDeleteAll(m_devices);
    m_devices.clear();
    m_servicesList.clear();
    qDebug() <<  "devices lengh: " << devices.size() << Qt::endl;
    for (int i = 0; i < devices.size(); i++) {
        DeviceInfoBluez *dev = new DeviceInfoBluez(devices[i]);
        mDevicesNames.append(QString("%1 [%2]").arg(dev->getName()).arg(dev->getAddress()));
        m_devices.append(dev);
    }
    emit devicesNamesChanged(mDevicesNames);
}

void BLEInterfaceBluez::readCharacteristicByService(const std::shared_ptr<SimpleBluez::Service> &service )
{
    for (auto charac : service->characteristics()) {
        std::string value = charac->read();
        auto battery_value = hex2dec(QString::fromStdString(value.c_str()).toUtf8().toHex().toStdString());
        qDebug() << "Characteristic uuid: " << charac->uuid();
        qDebug() << "Characteristic contents: " << battery_value;
        try {
            auto descp = charac->get_descriptor(

                QBluetoothUuid((QBluetoothUuid::DescriptorType::CharacteristicUserDescription)).toString(QUuid::WithoutBraces).toStdString()

            );
            QByteArray hex = QByteArray::fromHex(QString::fromStdString(descp->value()).toLatin1().toHex());
            QString str = QString::fromUtf8(hex);
            qDebug() << "descriptors : " << str;
            qDebug() << "descriptors : " << descp->uuid();

            this->hDeviceBatteryInfo[DeviceInfoBluez::BatteryType::Peripheral] = battery_value;
        } catch (SimpleBluez::Exception::DescriptorNotFoundException) {
            this->hDeviceBatteryInfo[DeviceInfoBluez::BatteryType::Central] = battery_value;
            continue;
        }
    }
    qDebug() << this->hDeviceBatteryInfo;
    emit dataReceivedBatteryInfo(hDeviceBatteryInfo);
}

std::vector<std::shared_ptr<SimpleBluez::Service>> BLEInterfaceBluez::findServicesByUuid(const QBluetoothUuid &qBluetoothUuid){
    std::vector<std::shared_ptr<SimpleBluez::Service>> services;
    if (!m_servicesList.empty()){
        for (int i = 0; i < m_servicesList.size(); i++) {
            if (QBluetoothUuid(QString::fromStdString(m_servicesList[i].first->uuid())) == qBluetoothUuid)
                services.push_back(m_servicesList[i].first);
        }
    }
    return services;
}

void BLEInterfaceBluez::scanDevicesBluez()
{
    mDevicesNames.clear();
    qDeleteAll(m_devices);
    m_devices.clear();
    m_servicesList.clear();
    emit devicesNamesChanged(mDevicesNames);
    if (m_adapter_defualt != nullptr){
        SimpleBluez::Adapter::DiscoveryFilter filter;
        filter.Transport = SimpleBluez::Adapter::DiscoveryFilter::TransportType::LE;
        m_adapter_defualt->discovery_filter(filter);

        m_adapter_defualt->set_on_device_updated([this](std::shared_ptr<SimpleBluez::Device> device) {
            if (std::find(m_peripherals.begin(), m_peripherals.end(), device) == m_peripherals.end()) {
                m_peripherals.push_back(device);
            }
        });
        m_peripherals = m_adapter_defualt->device_paired_get();

        this->addDevices(m_peripherals);
    }
}


void BLEInterfaceBluez::setConfigCurrentDevice( ){
    if(m_devices.isEmpty())
        return;
    auto l_device = m_devices[mCurrentDevice]->getDevice();
    if (l_device  != nullptr){
        auto *dev = new DeviceInfoBluez(l_device);
        pref->setValue("current_device_name", QString("%1 [%2]").arg(dev->getName()).arg(dev->getAddress()));
        delete(dev);
    }
}

void BLEInterfaceBluez::disconnectDevice( ){
    if(m_devices.isEmpty())
        return;
    auto l_device = m_devices[ mCurrentDevice]->getDevice();
    if (l_device  != nullptr)
        l_device->disconnect();
    this->update_connected(false);
    pref->setValue("current_device_name", "");
}

void BLEInterfaceBluez::connectCurrentDevice()
{
    if(m_devices.isEmpty())
        return;
    auto cDevice = m_devices[ mCurrentDevice]->getDevice();
    qDebug() << "connecting to device: " << cDevice->name() << " [" << cDevice->address() << "]" << Qt::endl;
    try {
        cDevice->connect();
        emit statusInfoChanged(QString("Connected to %1.").arg(QString::fromStdString(cDevice->name())), true);
    } catch (SimpleDBus::Exception::SendFailed& e) {
        emit statusInfoChanged(QString("Connection %1 failed.").arg(QString::fromStdString(cDevice->name())), true);
        return;
    }

    if (cDevice->connected()){
        this->update_connected(true);
        emit dataConnectedToDevice(true);
        this->setConfigCurrentDevice();
    }

    m_servicesUuid.clear();
    mServices.clear();

    for (auto service : cDevice->services()) {
        mServices.append(QString::fromStdString(service->uuid()));
        m_servicesUuid.append(QBluetoothUuid(QString::fromStdString(service->uuid())));
        for (auto characteristic : service->characteristics()) {
            m_servicesList.push_back(std::make_pair(service, characteristic));
        }
    }

    if(m_servicesUuid.isEmpty()){
        emit statusInfoChanged("Can't find any services.", true);
        return;
    }

    this->readCharacteristicFromBatteryServices();
}

void BLEInterfaceBluez::readCharacteristicFromBatteryServices(){
    qDebug() << "starting read characteristics from battery services..." << Qt::endl;
    auto services = this->findServicesByUuid(QBluetoothUuid(QBluetoothUuid::ServiceClassUuid::BatteryService));
    qDebug() << "services found: " << services.size() << Qt::endl;
    if (!services.empty())
        for (const auto &s : services)
            this->readCharacteristicByService(s);
}

void BLEInterfaceBluez::checkDevicePiredUpdate() {
    qDebug() << "checking device status..." << Qt::endl;
    if (mConnectedDevice) {
        try {
            auto l_device = m_devices[mCurrentDevice]->getDevice();
            if (l_device  != nullptr){
                if (!l_device->connected()){
                    this->connectCurrentDevice();
                }
                else {
                    this->readCharacteristicFromBatteryServices();
                }
            }
        } catch (...) {
            return;
        }
    }
}

void BLEInterfaceBluez::loadConfigDevice() {
    if (!pref->checkIfEmpty("current_device_name"))
        if (mDevicesNames.contains(pref->value("current_device_name"))){
            qDebug() << "restore : " << mDevicesNames.indexOf(pref->value("current_device_name")) << "device: " << pref->value("current_device_name");
            this->set_currentDevice(mDevicesNames.indexOf(pref->value("current_device_name")));
            this->connectCurrentDevice();
        }
}

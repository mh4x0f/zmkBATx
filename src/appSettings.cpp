#include "appSettings.h"
#include <QSettings>
#include <QApplication>

AppSettings *AppSettings::s_instance = nullptr;

AppSettings *AppSettings::getInstance()
{
    if (s_instance == nullptr)
    {
        s_instance = new AppSettings();
    }

    return s_instance;
}

AppSettings::AppSettings()
{
    Q_ASSERT(s_instance == nullptr);
    s_instance = this;
    if (this->value("CONFIG_TIME_REPORT_INTERVAL").isNull())
        this->m_settings.setValue("CONFIG_TIME_REPORT_INTERVAL", 180000);
}

AppSettings::~AppSettings()
{
    Q_ASSERT(s_instance != nullptr);
    s_instance = nullptr;
}

bool AppSettings::contains(const QString &key)
{
    return s_instance->m_settings.contains(key);
}

QVariant AppSettings::value(const QString &key, const QVariant &defaultValue)
{
    return s_instance->m_settings.value(key, defaultValue);
}

void AppSettings::setValue(const QString &key, const QVariant &value)
{
    s_instance->m_settings.setValue(key, value);
}

bool AppSettings::checkIfEmpty(const QString &key)
{
    return s_instance->m_settings.value(key).toString().isEmpty();
}

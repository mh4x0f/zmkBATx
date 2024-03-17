#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QSettings>

/// Singleton! Create only one instance!
class AppSettings
{
public:
    AppSettings();
    ~AppSettings();

    static AppSettings *getInstance();

    static bool contains(const QString &key);
    static QVariant value(const QString &key, const QVariant &defaultValue = QVariant());
    static void setValue(const QString &key, const QVariant &value);
    static bool checkIfEmpty(const QString &key);

private:
    static AppSettings *s_instance;
    QSettings m_settings;
};


#endif // APPSETTINGS_H

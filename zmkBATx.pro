

QT     += core gui bluetooth
CONFIG += c++17

QT += widgets

# Validate Qt version
!versionAtLeast(QT_VERSION, 6.2) : error("You need at least Qt version 6.2 for $${TARGET}")

TARGET = zmkBATx
TEMPLATE = app

SOURCES += src/main.cpp\
    src/about.cpp \
    src/appSettings.cpp \
    src/bleinterfacebluez.cpp \
        src/mainwindow.cpp

HEADERS  += src/mainwindow.h \
    src/about.h \
    src/appSettings.h \
    src/bleinterfacebluez.h

INCLUDEPATH += src/ src/thirdparty/

FORMS    += src/forms/mainwindow.ui \
    src/forms/about.ui

RESOURCES += \
    resources.qrc


OTHER_FILES += README.md \
               deploy_dep.sh \
               .gitignore

LIBS += -L$$PWD/src/thirdparty/ -lsimplebluez

INCLUDEPATH += $$PWD/src/thirdparty
DEPENDPATH += $$PWD/src/thirdparty

# Inclui o diretório do D-Bus
INCLUDEPATH += /usr/include/dbus-1.0

# Inclui o diretório para o mecanismo de sistema de mensagem D-Bus
INCLUDEPATH += /usr/lib/x86_64-linux-gnu/dbus-1.0/include

# Link com a biblioteca D-Bus
LIBS += -ldbus-1


# Build artifacts ##############################################################

OBJECTS_DIR = build/$${QT_ARCH}/
MOC_DIR     = build/$${QT_ARCH}/
RCC_DIR     = build/$${QT_ARCH}/

UI_DIR = $$PWD/src/forms

DESTDIR     = bin/



linux:!android {
    TARGET = $$lower($${TARGET})

    # Installation steps
    isEmpty(PREFIX) { PREFIX = /usr/local }
    target_app.files       += $${OUT_PWD}/$${DESTDIR}/$$lower($${TARGET})
    target_app.path         = $${PREFIX}/bin/
    target_appentry.files  += $${OUT_PWD}/assets/linux/$$lower($${TARGET}).desktop
    target_appentry.path    = $${PREFIX}/share/applications
    target_appdata.files   += $${OUT_PWD}/assets/linux/$$lower($${TARGET}).appdata.xml
    target_appdata.path     = $${PREFIX}/share/appdata

    INSTALLS += target_app target_appentry target_appdata

}


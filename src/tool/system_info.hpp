#pragma once
#include "tool/game_path.hpp"

#include <QCoreApplication>
#include <QObject>
#include <QQmlEngine>
#include <QSysInfo>
class SystemInfo : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(QString productType READ productType CONSTANT)
    Q_PROPERTY(QString productVersion READ productVersion CONSTANT)
    Q_PROPERTY(QString kernelType READ kernelType CONSTANT)
    Q_PROPERTY(QString kernelVersion READ kernelVersion CONSTANT)
    Q_PROPERTY(QString machineUniqueId READ machineUniqueId CONSTANT)
    Q_PROPERTY(QString qtVersion READ qtVersion CONSTANT)
    Q_PROPERTY(
        QString defaultGameInstallDir READ defaultGameInstallDir CONSTANT)
    Q_PROPERTY(QString defaultFrpInstallDir READ defaultFrpInstallDir CONSTANT)
    Q_PROPERTY(QString defaultEasyTierInstallDir READ defaultEasyTierInstallDir
                   CONSTANT)
    Q_PROPERTY(bool isInChina READ isInChina CONSTANT)
public:
    QString productType() const { return QSysInfo::productType(); }
    QString productVersion() const { return QSysInfo::productVersion(); }
    QString kernelType() const { return QSysInfo::kernelType(); }
    QString kernelVersion() const { return QSysInfo::kernelVersion(); }
    QString machineUniqueId() const { return QSysInfo::machineUniqueId(); }
    QString qtVersion() const { return QString::fromLatin1(QT_VERSION_STR); }
    QString defaultGameInstallDir() const {
        return get_default_game_install_dir();
    }
    QString defaultFrpInstallDir() const {
        return get_default_frp_install_dir();
    }
    QString defaultEasyTierInstallDir() const {
        return get_default_easytier_install_dir();
    }

    bool isInChina() const {
        QLocale::Country current = QLocale::system().territory();
        return current == QLocale::China;
    }
};
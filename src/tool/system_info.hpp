#pragma once
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
    Q_PROPERTY(QString defaultGameFilePath READ defaultGameFilePath CONSTANT)
public:
    QString productType() const { return QSysInfo::productType(); }
    QString productVersion() const { return QSysInfo::productVersion(); }
    QString kernelType() const { return QSysInfo::kernelType(); }
    QString kernelVersion() const { return QSysInfo::kernelVersion(); }
    QString machineUniqueId() const { return QSysInfo::machineUniqueId(); }
    QString qtVersion() const { return QString::fromLatin1(QT_VERSION_STR); }
    QString defaultGameFilePath() const {
        QString game_file_path{QCoreApplication::applicationDirPath() +
                               "/game/"};
#ifdef _WIN32
        game_file_path.append("Cubed.exe");
#else
        game_file_path.append("Cubed");
#endif
        return game_file_path;
    }
};
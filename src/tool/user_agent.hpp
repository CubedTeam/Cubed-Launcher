#pragma once
#include <QCoreApplication>

inline QString buildUserAgent() {

    QString app_name = QCoreApplication::applicationName();
    if (app_name.isEmpty())
        app_name = "Qt-UpdateChecker";

    QString app_ver = QCoreApplication::applicationVersion();
    if (app_ver.isEmpty())
        app_ver = "1.0";

    return QString("%1/%2 (Qt/%3; %4; %5)")
        .arg(app_name)
        .arg(app_ver)
        .arg(qVersion())
        .arg(QSysInfo::prettyProductName())
        .arg(QSysInfo::currentCpuArchitecture());
}
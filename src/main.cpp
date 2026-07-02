#include "version.hpp"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>

int main(int argc, char** argv) {

    QGuiApplication app(argc, argv);

    QQuickStyle::setStyle("Material");
    QString version(APP_VERSION);
#ifdef _WIN32
    version.append("-windows");
#elif defined(__linux__)
    version.append("-linux");
#endif
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("AppVersion", version);
    engine.loadFromModule("CubedLauncher", "Main");

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }
    return app.exec();
}
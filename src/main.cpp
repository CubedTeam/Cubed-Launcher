#include "version.hpp"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>

int main(int argc, char** argv) {

    QGuiApplication app(argc, argv);

    QQuickStyle::setStyle("Material");

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("AppVersion", APP_VERSION);
    engine.loadFromModule("CubedLauncher", "Main");

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }
    return app.exec();
}
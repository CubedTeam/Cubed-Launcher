#include "settings.hpp"
#include "version.hpp"

#include <QGuiApplication>
#include <QLocale>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QSettings>
#include <QTranslator>

// AI-generated: live language switch state shared with Settings callbacks.
static QTranslator* g_translator = nullptr;
static QQmlApplicationEngine* g_engine = nullptr;
static QString g_current_language;

static void apply_language(const QString& lang) {
    if (lang == g_current_language) {
        return;
    }
    if (g_translator) {
        qApp->removeTranslator(g_translator);
    }
    if (g_translator && g_translator->load("CubedLauncher_" + lang, ":/i18n")) {
        qApp->installTranslator(g_translator);
    }
    g_current_language = lang;
    if (g_engine) {
        g_engine->retranslate();
    }
}

int main(int argc, char** argv) {

    QGuiApplication app(argc, argv);

    QCoreApplication::setApplicationName("CubedLauncher");

    QSettings cfg(QSettings::IniFormat, QSettings::UserScope, "Cubed",
                  "Launcher");
    QString lang = cfg.value("language").toString();
    if (lang.isEmpty()) {
        const bool in_china = QLocale::system().territory() == QLocale::China;
        lang = in_china ? QStringLiteral("zh_CN") : QStringLiteral("en");
        cfg.setValue("language", lang);
    }
    g_current_language = lang;

    QTranslator translator;
    g_translator = &translator;
    if (translator.load("CubedLauncher_" + lang, ":/i18n")) {
        app.installTranslator(&translator);
    }

    QQuickStyle::setStyle("Material");
    QString version(APP_VERSION);
#ifdef _WIN32
    version.append("-windows");
#elif defined(__linux__)
    version.append("-linux");
#endif
    QQmlApplicationEngine engine;
    g_engine = &engine;
    engine.rootContext()->setContextProperty("AppVersion", version);
    engine.loadFromModule("CubedLauncher", "Main");

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    // AI-generated: react to Settings::languageChanged by retranslating QML.
    if (Settings* s = Settings::instance()) {
        QObject::connect(s, &Settings::language_changed, s, []() {
            apply_language(Settings::instance()->language());
        });
    }

    return app.exec();
}
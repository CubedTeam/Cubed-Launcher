#pragma once

#include <QColor>
#include <QObject>
#include <QQmlEngine>
#include <QString>

class AnsiTextFormatter : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit AnsiTextFormatter(QObject* parent = nullptr);

    Q_INVOKABLE QString to_html(const QString& text,
                                const QColor& default_foreground,
                                const QColor& default_background) const;
};

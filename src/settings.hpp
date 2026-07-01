#pragma once
#include <QObject>
#include <QQmlEngine>
#include <QSettings>
class Settings : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(QString gamePath READ game_path WRITE set_game_path NOTIFY
                   game_path_changed FINAL)
    Q_PROPERTY(QString playerName READ player_name WRITE set_player_name NOTIFY
                   player_name_changed FINAL)
    Q_PROPERTY(bool pathSetted READ path_set NOTIFY path_set_changed FINAL)
public:
    explicit Settings(QObject* parent = nullptr);
    QString game_path() const;
    QString player_name() const;
    bool path_set() const;
public slots:
    void set_game_path(const QUrl& path);
    void set_player_name(const QString& name);
signals:
    void game_path_changed();
    void player_name_changed();
    void path_set_changed();

private:
    QSettings m_settings;
    QString m_game_path;
    QString m_player_name;
    void load();
    void save(const QString& key, const QString& value);

    template <typename T>
    bool update_value(T& member, const T& value, const QString& key) {
        if (member == value)
            return false;

        member = value;
        m_settings.setValue(key, QVariant::fromValue(value));
        return true;
    }
};
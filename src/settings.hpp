#pragma once

#include <QColor>
#include <QObject>
#include <QQmlEngine>
#include <QSettings>
class Settings : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(QString gameDir READ game_dir WRITE set_game_dir_url NOTIFY
                   game_dir_changed FINAL)
    Q_PROPERTY(QString playerName READ player_name WRITE set_player_name NOTIFY
                   player_name_changed FINAL)
    Q_PROPERTY(bool pathSetted READ path_set NOTIFY path_set_changed FINAL)
    Q_PROPERTY(int mirrorIndex READ mirror_index WRITE set_mirror_index NOTIFY
                   mirror_index_changed FINAL)
    Q_PROPERTY(QString language READ language WRITE set_language NOTIFY
                   language_changed FINAL)
    Q_PROPERTY(QColor accentColor READ accent_color WRITE set_accent_color
                   NOTIFY accent_color_changed FINAL)
    Q_PROPERTY(
        bool cardColorfulBorder READ card_colorful_border WRITE
            set_card_colorful_border NOTIFY card_colorful_border_changed FINAL)
    Q_PROPERTY(QString wrapperCommand READ wrapper_command WRITE
                   set_wrapper_command NOTIFY wrapper_command_changed FINAL)
public:
    explicit Settings(QObject* parent = nullptr);
    QString game_dir() const;
    QString player_name() const;
    bool path_set() const;
    int mirror_index() const;
    QString language() const;
    QColor accent_color() const;
    bool card_colorful_border() const;
    QString wrapper_command() const;
    static Settings* instance();
public slots:
    void set_game_dir_url(const QUrl& path);
    void set_game_dir(const QString& path);
    void set_player_name(const QString& name);
    void set_mirror_index(int index);
    void set_language(const QString& lang);
    void set_accent_color(const QColor& color);
    void set_card_colorful_border(bool enabled);
    void set_wrapper_command(const QString& command);
signals:
    void game_dir_changed();
    void player_name_changed();
    void path_set_changed();
    void mirror_index_changed();
    void language_changed();
    void accent_color_changed();
    void card_colorful_border_changed();
    void wrapper_command_changed();

private:
    QSettings m_settings;
    QString m_game_dir;
    QString m_player_name;
    int m_mirror_index{-1};
    QString m_language;
    QString m_wrapper_command;

    QColor m_accent_color;
    bool m_card_colorful_border{true};
    static Settings* s_instance;
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

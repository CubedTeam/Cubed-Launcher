#pragma once
#include <QColor>
#include <QObject>
#include <QQmlEngine>
#include <QSettings>
class Settings : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(QString gamePath READ game_path WRITE set_game_path_url NOTIFY
                   game_path_changed FINAL)
    Q_PROPERTY(QString playerName READ player_name WRITE set_player_name NOTIFY
                   player_name_changed FINAL)
    Q_PROPERTY(bool pathSetted READ path_set NOTIFY path_set_changed FINAL)
    // AI-generated: selected mirror index, -1 means unset.
    Q_PROPERTY(int mirrorIndex READ mirror_index WRITE set_mirror_index NOTIFY
                   mirror_index_changed FINAL)
    // AI-generated: UI language code, e.g. "zh_CN" / "en".
    Q_PROPERTY(QString language READ language WRITE set_language NOTIFY
                   language_changed FINAL)
    // AI-generated: global theme accent color, persisted and bound to Material.
    Q_PROPERTY(QColor accentColor READ accent_color WRITE set_accent_color
                   NOTIFY accent_color_changed FINAL)
    // AI-generated: whether Card borders use the accent color (true) or a
    // neutral grey (false).
    Q_PROPERTY(
        bool cardColorfulBorder READ card_colorful_border WRITE
            set_card_colorful_border NOTIFY card_colorful_border_changed FINAL)
public:
    explicit Settings(QObject* parent = nullptr);
    QString game_path() const;
    QString player_name() const;
    bool path_set() const;
    int mirror_index() const;
    // AI-generated: access the live QML-singleton instance from C++.
    QString language() const;
    // AI-generated: current theme accent color.
    QColor accent_color() const;
    // AI-generated: whether Card borders use the accent color.
    bool card_colorful_border() const;
    static Settings* instance();
public slots:
    void set_game_path_url(const QUrl& path);
    void set_game_path(const QString& path);
    void set_player_name(const QString& name);
    // AI-generated: persist selected mirror index.
    void set_mirror_index(int index);
    // AI-generated: persist chosen UI language code.
    void set_language(const QString& lang);
    // AI-generated: update and persist the theme accent color.
    void set_accent_color(const QColor& color);
    // AI-generated: persist and toggle colorful card borders.
    void set_card_colorful_border(bool enabled);
signals:
    void game_path_changed();
    void player_name_changed();
    void path_set_changed();
    void mirror_index_changed();
    void language_changed();
    // AI-generated: emitted when the accent color changes.
    void accent_color_changed();
    // AI-generated: emitted when the colorful card border toggle changes.
    void card_colorful_border_changed();

private:
    QSettings m_settings;
    QString m_game_path;
    QString m_player_name;
    // AI-generated: -1 means unset.
    int m_mirror_index{-1};
    // AI-generated: empty means not yet configured.
    QString m_language;

    QColor m_accent_color;
    // AI-generated: when true, Card border uses accent color; otherwise neutral
    // grey.
    bool m_card_colorful_border{true};
    // AI-generated: tracks the single QML-singleton instance.
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
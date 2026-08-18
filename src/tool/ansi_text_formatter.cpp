#include "tool/ansi_text_formatter.hpp"

#include <QList>
#include <QStringList>
#include <array>
#include <optional>
#include <utility>

namespace {

// AI-generated: Render ANSI SGR output as safe Qt rich text.
struct TextStyle {
    std::optional<QColor> foreground;
    std::optional<QColor> background;
    bool bold{false};
    bool dim{false};
    bool italic{false};
    bool underline{false};
    bool reverse{false};
    bool strike{false};
};

QColor ansi_color(int index) {
    static const std::array<QColor, 16> basic_colors = {
        QColor(QStringLiteral("#000000")), QColor(QStringLiteral("#800000")),
        QColor(QStringLiteral("#008000")), QColor(QStringLiteral("#808000")),
        QColor(QStringLiteral("#000080")), QColor(QStringLiteral("#800080")),
        QColor(QStringLiteral("#008080")), QColor(QStringLiteral("#c0c0c0")),
        QColor(QStringLiteral("#808080")), QColor(QStringLiteral("#ff0000")),
        QColor(QStringLiteral("#00ff00")), QColor(QStringLiteral("#ffff00")),
        QColor(QStringLiteral("#0000ff")), QColor(QStringLiteral("#ff00ff")),
        QColor(QStringLiteral("#00ffff")), QColor(QStringLiteral("#ffffff")),
    };

    if (index < 0 || index > 255) {
        return {};
    }
    if (index < static_cast<int>(basic_colors.size())) {
        return basic_colors[static_cast<std::size_t>(index)];
    }
    if (index < 232) {
        static constexpr std::array<int, 6> levels = {0,   95,  135,
                                                      175, 215, 255};
        const int cube_index = index - 16;
        return QColor(levels[static_cast<std::size_t>(cube_index / 36)],
                      levels[static_cast<std::size_t>((cube_index / 6) % 6)],
                      levels[static_cast<std::size_t>(cube_index % 6)]);
    }

    const int gray = 8 + (index - 232) * 10;
    return QColor(gray, gray, gray);
}

std::optional<int> parse_parameter(QStringView parameter) {
    if (parameter.isEmpty()) {
        return 0;
    }
    bool ok = false;
    const int value = parameter.toInt(&ok);
    if (!ok) {
        return std::nullopt;
    }
    return value;
}

void apply_basic_parameter(int parameter, TextStyle& style) {
    if (parameter == 0) {
        style = {};
    } else if (parameter == 1) {
        style.bold = true;
    } else if (parameter == 2) {
        style.dim = true;
    } else if (parameter == 3) {
        style.italic = true;
    } else if (parameter == 4) {
        style.underline = true;
    } else if (parameter == 7) {
        style.reverse = true;
    } else if (parameter == 9) {
        style.strike = true;
    } else if (parameter == 22) {
        style.bold = false;
        style.dim = false;
    } else if (parameter == 23) {
        style.italic = false;
    } else if (parameter == 24) {
        style.underline = false;
    } else if (parameter == 27) {
        style.reverse = false;
    } else if (parameter == 29) {
        style.strike = false;
    } else if (parameter >= 30 && parameter <= 37) {
        style.foreground = ansi_color(parameter - 30);
    } else if (parameter == 39) {
        style.foreground.reset();
    } else if (parameter >= 40 && parameter <= 47) {
        style.background = ansi_color(parameter - 40);
    } else if (parameter == 49) {
        style.background.reset();
    } else if (parameter >= 90 && parameter <= 97) {
        style.foreground = ansi_color(parameter - 90 + 8);
    } else if (parameter >= 100 && parameter <= 107) {
        style.background = ansi_color(parameter - 100 + 8);
    }
}

void set_extended_color(int selector, int mode, const QList<int>& values,
                        TextStyle& style) {
    QColor color;
    if (mode == 5 && !values.isEmpty()) {
        color = ansi_color(values.constFirst());
    } else if (mode == 2 && values.size() >= 3) {
        const qsizetype offset = values.size() - 3;
        const int red = values[offset];
        const int green = values[offset + 1];
        const int blue = values[offset + 2];
        if (red >= 0 && red <= 255 && green >= 0 && green <= 255 && blue >= 0 &&
            blue <= 255) {
            color = QColor(red, green, blue);
        }
    }

    if (!color.isValid()) {
        return;
    }
    if (selector == 38) {
        style.foreground = color;
    } else {
        style.background = color;
    }
}

void apply_colon_parameter(const QString& group, TextStyle& style) {
    const QStringList parts = group.split(QLatin1Char(':'), Qt::KeepEmptyParts);
    if (parts.isEmpty()) {
        return;
    }
    const auto selector = parse_parameter(parts.constFirst());
    if (!selector) {
        return;
    }
    if ((*selector != 38 && *selector != 48) || parts.size() < 2) {
        apply_basic_parameter(*selector, style);
        return;
    }

    const auto mode = parse_parameter(parts[1]);
    if (!mode) {
        return;
    }
    QList<int> values;
    for (qsizetype i = 2; i < parts.size(); ++i) {
        if (parts[i].isEmpty()) {
            continue;
        }
        const auto value = parse_parameter(parts[i]);
        if (!value) {
            return;
        }
        values.push_back(*value);
    }
    set_extended_color(*selector, *mode, values, style);
}

void apply_sgr(QStringView parameters, TextStyle& style) {
    const QStringList groups =
        parameters.toString().split(QLatin1Char(';'), Qt::KeepEmptyParts);
    for (qsizetype i = 0; i < groups.size(); ++i) {
        if (groups[i].contains(QLatin1Char(':'))) {
            apply_colon_parameter(groups[i], style);
            continue;
        }

        const auto parameter = parse_parameter(groups[i]);
        if (!parameter) {
            continue;
        }
        if ((*parameter == 38 || *parameter == 48) && i + 1 < groups.size()) {
            const auto mode = parse_parameter(groups[i + 1]);
            if (mode && *mode == 5 && i + 2 < groups.size()) {
                const auto color_index = parse_parameter(groups[i + 2]);
                if (color_index) {
                    set_extended_color(*parameter, *mode, {*color_index},
                                       style);
                }
                i += 2;
                continue;
            }
            if (mode && *mode == 2 && i + 4 < groups.size()) {
                QList<int> values;
                for (qsizetype value_index = i + 2; value_index <= i + 4;
                     ++value_index) {
                    const auto value = parse_parameter(groups[value_index]);
                    if (!value) {
                        values.clear();
                        break;
                    }
                    values.push_back(*value);
                }
                set_extended_color(*parameter, *mode, values, style);
                i += 4;
                continue;
            }
        }
        apply_basic_parameter(*parameter, style);
    }
}

QColor blend(const QColor& foreground, const QColor& background) {
    constexpr qreal foreground_weight = 0.6;
    constexpr qreal background_weight = 1.0 - foreground_weight;
    return QColor::fromRgbF(foreground.redF() * foreground_weight +
                                background.redF() * background_weight,
                            foreground.greenF() * foreground_weight +
                                background.greenF() * background_weight,
                            foreground.blueF() * foreground_weight +
                                background.blueF() * background_weight);
}

QString style_css(const TextStyle& style, const QColor& default_foreground,
                  const QColor& default_background) {
    QColor foreground = style.foreground.value_or(default_foreground);
    QColor background = style.background.value_or(default_background);
    if (style.reverse) {
        std::swap(foreground, background);
    }
    if (style.dim) {
        foreground = blend(foreground, background);
    }

    QStringList declarations;
    if (style.foreground || style.dim || style.reverse) {
        declarations.push_back(
            QStringLiteral("color:%1").arg(foreground.name(QColor::HexRgb)));
    }
    if (style.background || style.reverse) {
        declarations.push_back(QStringLiteral("background-color:%1")
                                   .arg(background.name(QColor::HexRgb)));
    }
    if (style.bold) {
        declarations.push_back(QStringLiteral("font-weight:700"));
    }
    if (style.italic) {
        declarations.push_back(QStringLiteral("font-style:italic"));
    }
    if (style.underline || style.strike) {
        QStringList decorations;
        if (style.underline) {
            decorations.push_back(QStringLiteral("underline"));
        }
        if (style.strike) {
            decorations.push_back(QStringLiteral("line-through"));
        }
        declarations.push_back(QStringLiteral("text-decoration:%1")
                                   .arg(decorations.join(QLatin1Char(' '))));
    }
    return declarations.join(QLatin1Char(';'));
}

void append_segment(QString& html, QString& segment, const TextStyle& style,
                    const QColor& default_foreground,
                    const QColor& default_background) {
    if (segment.isEmpty()) {
        return;
    }
    const QString escaped = segment.toHtmlEscaped();
    const QString css =
        style_css(style, default_foreground, default_background);
    if (css.isEmpty()) {
        html.append(escaped);
    } else {
        html.append(
            QStringLiteral("<span style=\"%1\">%2</span>").arg(css, escaped));
    }
    segment.clear();
}

qsizetype find_csi_end(QStringView text, qsizetype start) {
    for (qsizetype i = start; i < text.size(); ++i) {
        const ushort code = text[i].unicode();
        if (code >= 0x40 && code <= 0x7e) {
            return i;
        }
    }
    return -1;
}

qsizetype find_osc_end(QStringView text, qsizetype start) {
    for (qsizetype i = start; i < text.size(); ++i) {
        if (text[i].unicode() == 0x07 || text[i].unicode() == 0x009c) {
            return i;
        }
        if (text[i].unicode() == 0x1b && i + 1 < text.size() &&
            text[i + 1] == QLatin1Char('\\')) {
            return i + 1;
        }
    }
    return -1;
}

} // namespace

AnsiTextFormatter::AnsiTextFormatter(QObject* parent) : QObject(parent) {}

QString AnsiTextFormatter::to_html(const QString& text,
                                   const QColor& default_foreground,
                                   const QColor& default_background) const {
    const QColor foreground = default_foreground.isValid()
                                  ? default_foreground
                                  : QColor(QStringLiteral("#b8e8ff"));
    const QColor background = default_background.isValid()
                                  ? default_background
                                  : QColor(QStringLiteral("#101418"));
    QString html = QStringLiteral("<pre style=\"margin:0;color:%1\">")
                       .arg(foreground.name(QColor::HexRgb));
    QString segment;
    TextStyle style;
    const QStringView view(text);

    for (qsizetype i = 0; i < view.size();) {
        const ushort code = view[i].unicode();
        const bool escape_csi = code == 0x1b && i + 1 < view.size() &&
                                view[i + 1] == QLatin1Char('[');
        const bool c1_csi = code == 0x009b;
        if (escape_csi || c1_csi) {
            append_segment(html, segment, style, foreground, background);
            const qsizetype parameter_start = i + (escape_csi ? 2 : 1);
            const qsizetype end = find_csi_end(view, parameter_start);
            if (end < 0) {
                break;
            }
            if (view[end] == QLatin1Char('m')) {
                apply_sgr(view.sliced(parameter_start, end - parameter_start),
                          style);
            }
            i = end + 1;
            continue;
        }

        const bool escape_osc = code == 0x1b && i + 1 < view.size() &&
                                view[i + 1] == QLatin1Char(']');
        const bool c1_osc = code == 0x009d;
        if (escape_osc || c1_osc) {
            append_segment(html, segment, style, foreground, background);
            const qsizetype end = find_osc_end(view, i + (escape_osc ? 2 : 1));
            if (end < 0) {
                break;
            }
            i = end + 1;
            continue;
        }

        if (code == 0x1b) {
            append_segment(html, segment, style, foreground, background);
            i += i + 1 < view.size() ? 2 : 1;
            continue;
        }
        if ((code < 0x20 && code != '\n' && code != '\t') || code == 0x7f ||
            (code >= 0x80 && code <= 0x9f)) {
            ++i;
            continue;
        }

        segment.append(view[i]);
        ++i;
    }

    append_segment(html, segment, style, foreground, background);
    html.append(QStringLiteral("</pre>"));
    return html;
}

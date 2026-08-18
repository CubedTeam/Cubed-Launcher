#include "tool/ansi_text_formatter.hpp"

#include <QTextDocument>
#include <QtTest>

class AnsiTextFormatterTest : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void preservesPlainTextAndEscapesHtml();
    void rendersBasicColorsAndResets();
    void rendersExtendedColors();
    void rendersAndClearsTextStyles();
    void keepsStylesAcrossLines();
    void removesUnsupportedControlSequences();
    void ignoresInvalidColorParameters();

private:
    static QString format(const QString& text);
    static QString visibleText(const QString& html);
};

QString AnsiTextFormatterTest::format(const QString& text) {
    AnsiTextFormatter formatter;
    return formatter.to_html(text, QColor(QStringLiteral("#b8e8ff")),
                             QColor(QStringLiteral("#101418")));
}

QString AnsiTextFormatterTest::visibleText(const QString& html) {
    QTextDocument document;
    document.setHtml(html);
    return document.toPlainText();
}

void AnsiTextFormatterTest::preservesPlainTextAndEscapesHtml() {
    const QString input = QStringLiteral("<tag>&  two\tspaces\nnext");
    const QString html = format(input);

    QVERIFY(html.contains(QStringLiteral("&lt;tag&gt;&amp;")));
    QVERIFY(!html.contains(QStringLiteral("<tag>")));
    QCOMPARE(visibleText(html), input);
}

void AnsiTextFormatterTest::rendersBasicColorsAndResets() {
    const QString input = QStringLiteral(
        "\x1b[31mred\x1b[44m on blue\x1b[39m default\x1b[49m done "
        "\x1b[91mbright \x1b[34mblue \x1b[94mbright blue");
    const QString html = format(input);

    QVERIFY(html.contains(QStringLiteral("color:#800000")));
    QVERIFY(html.contains(QStringLiteral("background-color:#000080")));
    QVERIFY(html.contains(QStringLiteral("color:#ff0000")));
    QVERIFY(html.contains(QStringLiteral("color:#58a6ff")));
    QVERIFY(html.contains(QStringLiteral("color:#79c0ff")));
    QCOMPARE(
        visibleText(html),
        QStringLiteral("red on blue default done bright blue bright blue"));
}

void AnsiTextFormatterTest::rendersExtendedColors() {
    const QString html = format(QStringLiteral(
        "\x1b[38;5;196mred\x1b[48:5:244mgray\x1b[38;2;18;52;86mtrue"
        "\x1b[48:2::1:2:3mcolor"));

    QVERIFY(html.contains(QStringLiteral("color:#ff0000")));
    QVERIFY(html.contains(QStringLiteral("background-color:#808080")));
    QVERIFY(html.contains(QStringLiteral("color:#123456")));
    QVERIFY(html.contains(QStringLiteral("background-color:#010203")));
    QCOMPARE(visibleText(html), QStringLiteral("redgraytruecolor"));
}

void AnsiTextFormatterTest::rendersAndClearsTextStyles() {
    const QString html = format(
        QStringLiteral("\x1b[1;2;3;4;7;9mstyled\x1b[22;23;24;27;29mplain"));

    QVERIFY(html.contains(QStringLiteral("font-weight:700")));
    QVERIFY(html.contains(QStringLiteral("font-style:italic")));
    QVERIFY(html.contains(
        QStringLiteral("text-decoration:underline line-through")));
    QVERIFY(html.count(QStringLiteral("color:#")) >= 2);
    QVERIFY(html.contains(QStringLiteral("background-color:#b8e8ff")));
    QCOMPARE(visibleText(html), QStringLiteral("styledplain"));
}

void AnsiTextFormatterTest::keepsStylesAcrossLines() {
    const QString input = QStringLiteral("\x1b[32mfirst\nsecond\x1b[0m\nplain");
    const QString html = format(input);

    QCOMPARE(html.count(QStringLiteral("color:#008000")), 1);
    QCOMPARE(visibleText(html), QStringLiteral("first\nsecond\nplain"));
}

void AnsiTextFormatterTest::removesUnsupportedControlSequences() {
    const QString input =
        QStringLiteral("a\x1b[2Kb\x1b]0;window title\ac\x1b[999md\x1b[31");
    const QString html = format(input);

    QVERIFY(!html.contains(QChar(0x1b)));
    QCOMPARE(visibleText(html), QStringLiteral("abcd"));
}

void AnsiTextFormatterTest::ignoresInvalidColorParameters() {
    const QString html =
        format(QStringLiteral("\x1b[38;5;999mindexed\x1b[48;2;256;0;0mrgb"));

    QCOMPARE(html.count(QStringLiteral("color:#")), 1);
    QVERIFY(!html.contains(QStringLiteral("background-color:")));
    QCOMPARE(visibleText(html), QStringLiteral("indexedrgb"));
}

QTEST_MAIN(AnsiTextFormatterTest)

#include "ansi_text_formatter_test.moc"

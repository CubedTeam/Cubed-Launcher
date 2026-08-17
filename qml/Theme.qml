pragma Singleton
import QtQuick
import CubedLauncher

QtObject {
    function alpha(colorValue, opacity) {
        return Qt.rgba(colorValue.r, colorValue.g, colorValue.b, opacity);
    }

    readonly property bool resolvedDark: Settings.themeMode === 2
        || (Settings.themeMode === 0 && Application.styleHints.colorScheme === Qt.Dark)

    readonly property var palettes: ({
        cubed: {
            light: { primary: "#4B8003", onPrimary: "#FFFFFF", primaryContainer: "#C9F283", onPrimaryContainer: "#142000", secondary: "#586249", onSecondary: "#FFFFFF", secondaryContainer: "#DCE7C8", onSecondaryContainer: "#161E0B", tertiary: "#8C4F2B", onTertiary: "#FFFFFF", tertiaryContainer: "#FFDBCA", onTertiaryContainer: "#351000" },
            dark: { primary: "#AED066", onPrimary: "#233600", primaryContainer: "#344F00", onPrimaryContainer: "#C9F283", secondary: "#C0CBAE", onSecondary: "#2B331F", secondaryContainer: "#414A34", onSecondaryContainer: "#DCE7C8", tertiary: "#FFB692", onTertiary: "#542104", tertiaryContainer: "#713719", onTertiaryContainer: "#FFDBCA" }
        },
        blue: {
            light: { primary: "#0B57D0", onPrimary: "#FFFFFF", primaryContainer: "#D9E2FF", onPrimaryContainer: "#001A43", secondary: "#565E71", onSecondary: "#FFFFFF", secondaryContainer: "#DAE2F9", onSecondaryContainer: "#131C2B", tertiary: "#705575", onTertiary: "#FFFFFF", tertiaryContainer: "#FAD8FD", onTertiaryContainer: "#28132E" },
            dark: { primary: "#AFC6FF", onPrimary: "#002E6A", primaryContainer: "#004397", onPrimaryContainer: "#D9E2FF", secondary: "#BEC6DC", onSecondary: "#283141", secondaryContainer: "#3E4758", onSecondaryContainer: "#DAE2F9", tertiary: "#DDBCE1", onTertiary: "#3F2845", tertiaryContainer: "#573E5C", onTertiaryContainer: "#FAD8FD" }
        },
        violet: {
            light: { primary: "#6750A4", onPrimary: "#FFFFFF", primaryContainer: "#EADDFF", onPrimaryContainer: "#21005D", secondary: "#625B71", onSecondary: "#FFFFFF", secondaryContainer: "#E8DEF8", onSecondaryContainer: "#1D192B", tertiary: "#7D5260", onTertiary: "#FFFFFF", tertiaryContainer: "#FFD8E4", onTertiaryContainer: "#31111D" },
            dark: { primary: "#D0BCFF", onPrimary: "#381E72", primaryContainer: "#4F378B", onPrimaryContainer: "#EADDFF", secondary: "#CCC2DC", onSecondary: "#332D41", secondaryContainer: "#4A4458", onSecondaryContainer: "#E8DEF8", tertiary: "#EFB8C8", onTertiary: "#492532", tertiaryContainer: "#633B48", onTertiaryContainer: "#FFD8E4" }
        },
        teal: {
            light: { primary: "#006A6A", onPrimary: "#FFFFFF", primaryContainer: "#9CF1F0", onPrimaryContainer: "#002020", secondary: "#4A6363", onSecondary: "#FFFFFF", secondaryContainer: "#CCE8E7", onSecondaryContainer: "#051F1F", tertiary: "#4B607C", onTertiary: "#FFFFFF", tertiaryContainer: "#D3E4FF", onTertiaryContainer: "#041C35" },
            dark: { primary: "#80D5D4", onPrimary: "#003737", primaryContainer: "#004F4F", onPrimaryContainer: "#9CF1F0", secondary: "#B0CCCB", onSecondary: "#1B3534", secondaryContainer: "#324B4B", onSecondaryContainer: "#CCE8E7", tertiary: "#B3C8E8", onTertiary: "#1C314B", tertiaryContainer: "#334863", onTertiaryContainer: "#D3E4FF" }
        },
        orange: {
            light: { primary: "#9A4600", onPrimary: "#FFFFFF", primaryContainer: "#FFDBC7", onPrimaryContainer: "#321200", secondary: "#765846", onSecondary: "#FFFFFF", secondaryContainer: "#FFDBC7", onSecondaryContainer: "#2B170A", tertiary: "#626033", onTertiary: "#FFFFFF", tertiaryContainer: "#E8E4AC", onTertiaryContainer: "#1E1D00" },
            dark: { primary: "#FFB689", onPrimary: "#522300", primaryContainer: "#743500", onPrimaryContainer: "#FFDBC7", secondary: "#E6BEA7", onSecondary: "#432B1C", secondaryContainer: "#5C412F", onSecondaryContainer: "#FFDBC7", tertiary: "#CBC890", onTertiary: "#343209", tertiaryContainer: "#4A491E", onTertiaryContainer: "#E8E4AC" }
        }
    })

    readonly property var activePalette: palettes[Settings.themePalette] || palettes.cubed
    readonly property var scheme: resolvedDark ? activePalette.dark : activePalette.light

    readonly property color primary: scheme.primary
    readonly property color primaryForeground: scheme.onPrimary
    readonly property color primaryContainer: scheme.primaryContainer
    readonly property color primaryContainerForeground: scheme.onPrimaryContainer
    readonly property color secondary: scheme.secondary
    readonly property color secondaryForeground: scheme.onSecondary
    readonly property color secondaryContainer: scheme.secondaryContainer
    readonly property color secondaryContainerForeground: scheme.onSecondaryContainer
    readonly property color tertiary: scheme.tertiary
    readonly property color tertiaryForeground: scheme.onTertiary
    readonly property color tertiaryContainer: scheme.tertiaryContainer
    readonly property color tertiaryContainerForeground: scheme.onTertiaryContainer

    readonly property color error: resolvedDark ? "#FFB4AB" : "#BA1A1A"
    readonly property color errorForeground: resolvedDark ? "#690005" : "#FFFFFF"
    readonly property color errorContainer: resolvedDark ? "#93000A" : "#FFDAD6"
    readonly property color errorContainerForeground: resolvedDark ? "#FFDAD6" : "#410002"
    readonly property color surface: resolvedDark ? "#11140E" : "#FBFDF5"
    readonly property color surfaceContainerLow: resolvedDark ? "#1A1C17" : "#F5F7EF"
    readonly property color surfaceContainer: resolvedDark ? "#1E201B" : "#EFF1E9"
    readonly property color surfaceContainerHigh: resolvedDark ? "#282A25" : "#E9EBE3"
    readonly property color surfaceContainerHighest: resolvedDark ? "#333530" : "#E3E5DD"
    readonly property color surfaceForeground: resolvedDark ? "#E3E4DD" : "#1A1C18"
    readonly property color surfaceVariantForeground: resolvedDark ? "#C5C8BE" : "#45483F"
    readonly property color outline: resolvedDark ? "#8F9289" : "#75786F"
    readonly property color outlineVariant: resolvedDark ? "#45483F" : "#C5C8BE"
    readonly property color scrim: "#000000"

    readonly property int space4: 4
    readonly property int space8: 8
    readonly property int space12: 12
    readonly property int space16: 16
    readonly property int space24: 24
    readonly property int space32: 32
    readonly property int radiusSmall: 8
    readonly property int radiusMedium: 12
    readonly property int radiusLarge: 16
    readonly property int radiusExtraLarge: 24
    readonly property int bodySize: 14
    readonly property int bodyLargeSize: 16
    readonly property int titleSize: 20
    readonly property int headlineSize: 28
    readonly property int labelSize: 13
    readonly property int controlHeight: 44
    // AI-generated: Centralize motion timing.
    readonly property int motionFast: 150
    readonly property int motionNormal: 220
    readonly property int motionEmphasized: 300
    readonly property int motionEasing: Easing.OutCubic
    readonly property int motionExitEasing: Easing.InCubic
}

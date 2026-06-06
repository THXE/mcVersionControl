#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include <QString>
#include <QColor>

enum class Theme {
    Dark,
    Light
};

class ThemeManager : public QObject
{
    Q_OBJECT

public:
    static ThemeManager &instance();

    Theme currentTheme() const { return m_theme; }
    bool isDark() const { return m_theme == Theme::Dark; }
    bool isLight() const { return m_theme == Theme::Light; }

    static Theme systemTheme();
    QString loadThemeStyleSheet(Theme theme);
    void applyTheme(Theme theme);
    void forceApplyTheme(Theme theme);  // always sets palette+QSS, no guard
    void syncWithSystem();

signals:
    void themeChanged(Theme newTheme);

private:
    ThemeManager();
    ~ThemeManager() override = default;
    ThemeManager(const ThemeManager &) = delete;
    ThemeManager &operator=(const ThemeManager &) = delete;

    Theme m_theme = Theme::Dark;
};

/// Theme-aware color accessors — use these everywhere instead of hardcoded QColor
namespace ThemeColors {
    inline QColor bgMain()   { return ThemeManager::instance().isDark() ? QColor("#191919") : QColor("#F3F3F3"); }
    inline QColor bgPanel()  { return ThemeManager::instance().isDark() ? QColor("#202020") : QColor("#E8E8E8"); }
    inline QColor bgHover()  { return ThemeManager::instance().isDark() ? QColor("#2C2C2C") : QColor("#E5E5E5"); }
    inline QColor bgActive() { return ThemeManager::instance().isDark() ? QColor("#2C2C2C") : QColor("#E5E5E5"); }
    inline QColor border()   { return ThemeManager::instance().isDark() ? QColor("#636363") : QColor("#D1D1D1"); }
    inline QColor textMain() { return ThemeManager::instance().isDark() ? QColor("#DEDEDE") : QColor("#1A1A1A"); }
    inline QColor textSub()  { return ThemeManager::instance().isDark() ? QColor("#AAAAAA") : QColor("#555555"); }
    inline QColor textDim()  { return ThemeManager::instance().isDark() ? QColor("#888888") : QColor("#999999"); }
    inline QColor accent()   { return ThemeManager::instance().isDark() ? QColor("#555555") : QColor("#0078D4"); }
    inline QColor bgMainTrans() { return ThemeManager::instance().isDark() ? QColor(25,25,25,220) : QColor(243,243,243,230); }
    inline QColor closeRed() { return QColor("#e81123"); }
}

#endif // THEMEMANAGER_H

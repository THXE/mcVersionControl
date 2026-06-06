#include "ThemeManager.h"

#include <QApplication>
#include <QFile>
#include <QSettings>
#include <QWidget>
#include <QPalette>
#include <QDebug>

ThemeManager &ThemeManager::instance()
{
    static ThemeManager inst;
    return inst;
}

ThemeManager::ThemeManager()
{
    m_theme = systemTheme();
}

Theme ThemeManager::systemTheme()
{
    QSettings reg(
        "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        QSettings::NativeFormat);
    int light = reg.value("AppsUseLightTheme", 1).toInt();
    return (light == 0) ? Theme::Dark : Theme::Light;
}

QString ThemeManager::loadThemeStyleSheet(Theme theme)
{
    QString qssPath = (theme == Theme::Dark)
        ? ":/style_dark.qss"
        : ":/style_light.qss";

    QFile file(qssPath);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qWarning() << "ThemeManager: Cannot open" << qssPath;
        return {};
    }
    QString content = QString::fromUtf8(file.readAll());
    file.close();
    return content;
}

static void setAppPalette(Theme theme)
{
    bool dark = (theme == Theme::Dark);
    QPalette pal;

    QColor base   = dark ? QColor("#191919") : QColor("#F3F3F3");
    QColor panel  = dark ? QColor("#202020") : QColor("#FAFAFA");
    QColor text   = dark ? QColor("#DEDEDE") : QColor("#1A1A1A");
    QColor hilite = dark ? QColor("#2C2C2C") : QColor("#E5E5E5");
    QColor border = dark ? QColor("#636363") : QColor("#D1D1D1");

    pal.setColor(QPalette::Window,          base);
    pal.setColor(QPalette::WindowText,      text);
    pal.setColor(QPalette::Base,            base);
    pal.setColor(QPalette::AlternateBase,   panel);
    pal.setColor(QPalette::ToolTipBase,     panel);
    pal.setColor(QPalette::ToolTipText,     text);
    pal.setColor(QPalette::Text,            text);
    pal.setColor(QPalette::Button,          panel);
    pal.setColor(QPalette::ButtonText,      text);
    pal.setColor(QPalette::BrightText,      text);
    pal.setColor(QPalette::Highlight,       hilite);
    pal.setColor(QPalette::HighlightedText, text);
    pal.setColor(QPalette::Light,           panel);
    pal.setColor(QPalette::Midlight,        border);
    pal.setColor(QPalette::Mid,             border);
    pal.setColor(QPalette::Dark,            base);
    pal.setColor(QPalette::Shadow,          QColor(0,0,0,80));

    qApp->setPalette(pal);
}

void ThemeManager::applyTheme(Theme theme)
{
    if (m_theme == theme) return;
    forceApplyTheme(theme);
}

void ThemeManager::forceApplyTheme(Theme theme)
{
    m_theme = theme;
    setAppPalette(theme);
    QString qss = loadThemeStyleSheet(theme);
    if (!qss.isEmpty()) {
        qApp->setStyleSheet(qss);
    }
    for (QWidget *w : qApp->allWidgets()) {
        w->update();
    }
    emit themeChanged(theme);
}

void ThemeManager::syncWithSystem()
{
    Theme sys = systemTheme();
    if (m_theme != sys) {
        applyTheme(sys);
    }
}

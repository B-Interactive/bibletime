/*********
*
* In the name of the Father, and of the Son, and of the Holy Spirit.
*
* This file is part of BibleTime's source code, https://bibletime.info/
*
* Copyright 1999-2021 by the BibleTime developers.
* The BibleTime source code is licensed under the GNU General Public License
* version 2.0.
*
**********/

#include "colormanager.h"

#include <map>
#include <QApplication>
#include <QColor>
#include <QDir>
#include <QFileInfo>
#include <QList>
#include <QPalette>
#include <QSettings>
#include <QStringList>
#include <QVariant>
#include <utility>
#include "../../util/btassert.h"
#include "../../util/directory.h"
#include "../../backend/managers/cdisplaytemplatemgr.h"


namespace ColorManager {
namespace {

inline bool darkMode()
{ return qApp->palette().color(QPalette::Base).value() < 128; }

using ColorMaps = std::map<QString, std::map<QString, QString> >;

ColorMaps createColorMaps() {
    namespace DU = util::directory;
    QDir::Filters const readableFileFilter(QDir::Files | QDir::Readable);
    QStringList const cssFilter("*.css");

    ColorMaps colorMaps;

    auto const loadColorMap =
            [&colorMaps](QString const & filePath) {
                QFileInfo const cssInfo(filePath);
                static QString const cMapPathTemplate("%1/%2.cmap");
                auto cMapPath(cMapPathTemplate.arg(cssInfo.path())
                                              .arg(cssInfo.completeBaseName()));
                QFileInfo const cMapInfo(cMapPath);
                auto fileName(cssInfo.fileName());

                // Start with default color map:
                std::map<QString, QString> colorMap;
                auto const p(qApp->palette());
                if (darkMode()) {
                    colorMap.emplace("FOREGROUND_COLOR",
                                     p.color(QPalette::WindowText).name());
                    colorMap.emplace("BACKGROUND_COLOR",
                                     p.color(QPalette::Base).name());
                    colorMap.emplace("HIGHLIGHT",
                                     QColor("#ffff00").name());
                    colorMap.emplace("BACKGROUND_HIGHLIGHT",
                                     QColor("#444466").name());
                    colorMap.emplace("CROSSREF_COLOR",
                                     QColor("#aac2ff").name());
                    colorMap.emplace("JESUS_WORDS_COLOR",
                                     QColor("#ff0000").name());
                } else {
                    colorMap.emplace("FOREGROUND_COLOR",
                                     p.color(QPalette::WindowText).name());
                    colorMap.emplace("BACKGROUND_COLOR",
                                     p.color(QPalette::Base).name());
                    colorMap.emplace("HIGHLIGHT",
                                     QColor("#ffff00").name());
                    colorMap.emplace("BACKGROUND_HIGHLIGHT",
                                     QColor("#ddddff").name());
                    colorMap.emplace("CROSSREF_COLOR",
                                     QColor("#1414ff").name());
                    colorMap.emplace("JESUS_WORDS_COLOR",
                                     QColor("#ff0000").name());
                }

                if (cMapInfo.exists()) {
                    QSettings cMapSettings(cMapPath, QSettings::IniFormat);
                    static QString const dark("dark");
                    static QString const light("light");
                    cMapSettings.beginGroup(darkMode() ? dark : light);
                    for (auto const & colorKey : cMapSettings.childKeys())
                        colorMap[colorKey] = cMapSettings.value(colorKey).toString();
                }
                colorMaps[std::move(fileName)] = std::move(colorMap);
            };

    // Load global app stylesheets
    auto const & td = DU::getDisplayTemplatesDir();
    for (auto const & file : td.entryList(cssFilter, readableFileFilter))
        loadColorMap(td.canonicalPath() + "/" + file);

    // Load user app stylesheets
    auto const & utd = DU::getUserDisplayTemplatesDir();
    for (auto const & file : utd.entryList(cssFilter, readableFileFilter))
        loadColorMap(utd.canonicalPath() + "/" + file);

    return colorMaps;
}

auto const & colorMaps() {
    static auto const maps = createColorMaps();
    return maps;
}

QString getColorByPattern(QString const & pattern, QString const & templateName)
{
    BT_ASSERT(!templateName.isEmpty());
    auto const & maps = colorMaps();
    auto const mapIt(maps.find(templateName));
    BT_ASSERT(mapIt != maps.end());
    auto const valueIt(mapIt->second.find(pattern));
    BT_ASSERT(valueIt != mapIt->second.end());
    BT_ASSERT(!valueIt->second.isEmpty());
    return valueIt->second;
}

} // anonymous namespace


QString replaceColors(QString content) {
    return replaceColors(std::move(content),
                         CDisplayTemplateMgr::activeTemplateName());
}

QString replaceColors(QString content, QString const & templateName) {
    static QString const pattern("#%1#");
    auto const & maps = colorMaps();
    auto const mapsIt = maps.find(templateName);
    BT_ASSERT(mapsIt != maps.end());
    for (auto const & [key, value] : mapsIt->second)
        content.replace(pattern.arg(key), value);
    return content;
}

QString getBackgroundColor()
{ return getBackgroundColor(CDisplayTemplateMgr::activeTemplateName()); }

QString getBackgroundColor(QString const & templateName)
{ return getColorByPattern("BACKGROUND_COLOR", templateName); }

QString getBackgroundHighlightColor() {
    return getBackgroundHighlightColor(
                CDisplayTemplateMgr::activeTemplateName());
}

QString getBackgroundHighlightColor(QString const & templateName)
{ return getColorByPattern("BACKGROUND_HIGHLIGHT", templateName); }

QString getForegroundColor()
{ return getForegroundColor(CDisplayTemplateMgr::activeTemplateName()); }

QString getForegroundColor(QString const & templateName)
{ return getColorByPattern("FOREGROUND_COLOR", templateName); }

QString getCrossRefColor()
{ return getCrossRefColor(CDisplayTemplateMgr::activeTemplateName()); }

QString getCrossRefColor(QString const & templateName)
{ return getColorByPattern("CROSSREF_COLOR", templateName); }

} // namespace ColorManager

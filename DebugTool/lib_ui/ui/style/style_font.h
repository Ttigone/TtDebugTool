#ifndef STYLE_FONT_H
#define STYLE_FONT_H

#include "base/basic_types.h"

#include <QFont>
#include <QFontMetrics>

namespace style {

QString SystemFontTag();
void SetCustomFont(const QString& font);

enum class FontFlag : uchar {
	Bold = 0x01,
	Italic = 0x02,
	Underline = 0x04,
	StrikeOut = 0x08,
	Semibold = 0x10,
	Monospace = 0x20,
};

namespace internal {

void StartFonts();

void DestoryFonts();

int RegisterFontFamily(const QString& family);

namespace
{

bool Started = false;

// 字体
std::vector<QString> FontFamilies;
// 字体索引
std::map<QString, int> FontFamilyIndices;
// Qt 字体关键字
std::map<uint64, uint32> QtFontKeys;
}


} // namespace internal


} // namespace style


#endif // STYLE_FONT_H
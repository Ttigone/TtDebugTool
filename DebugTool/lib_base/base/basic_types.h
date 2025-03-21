#ifndef BASIC_TYPES_H
#define BASIC_TYPES_H

#include <QtGlobal>
#include <QByteArray>
#include <QString>
#include <QStringList>

#include <string>
#include <memory>
#include <functional>
// #include <gsl/gsl>

// using gsl::not_null;


//using uchar = unsigned char; // Qt has uchar
using int8 = qint8;
using uint8 = quint8;
using int16 = qint16;
using uint16 = quint16;
using int32 = qint32;
using uint32 = quint32;
using int64 = qint64;
using uint64 = quint64;
using float32 = float;
using float64 = double;

using TimeId = int32;

enum Theme {
    day_custom,
    day_dark,
};


///
/// @brief The DateTimeFormat enum
/// FormatFull 完整日期和时间
/// FormatDateOnly 仅日期
/// FormatTileOnly 仅时间
/// FormatISO ISO 8601 格式
enum DateTimeFormat {
  FormatFull,
  FormatDateOnly,
  FormatTimeOnly,
  FormatISO,
};

// Q_ENUM(Theme);


/// @brief 将 UTF-16 编码的字符串字面量 转换为 QString 对象 _q 是自定义的字面量后缀, 标识特定操作
[[nodiscard]] inline QString operator""_q(const char16_t *data, std::size_t size)
{
    return QString::fromRawData(reinterpret_cast<const QChar *>(data), size);
}

#endif // BASIC_TYPES_H

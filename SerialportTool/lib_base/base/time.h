#ifndef LIB_BASE_TIME_H
#define LIB_BASE_TIME_H

// #include <QDateTime>

QString formatDateTime(const QDateTime& datatime, DateTimeFormat format) {
  switch (format) {
    case FormatFull:
      return datatime.toString("yyyy-MM-dd HH:mm:ss");
      break;
    case FormatDateOnly:
      return datatime.toString("yyyy-MM-dd");
      break;
    case FormatTimeOnly:
      return datatime.toString("HH:mm:ss");
      break;
    case FormatISO:
      return datatime.toString(Qt::ISODate);
      break;
    default:
      return datatime.toString();
  }
}

QString GetCurrentLocalTime(
    DateTimeFormat format = DateTimeFormat::FormatTimeOnly) {
  QDateTime dataTime = QDateTime::currentDateTime();
  return formatDateTime(dataTime, format);
}

#endif  // LIB_BASE_TIME_H

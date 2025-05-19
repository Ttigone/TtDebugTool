#ifndef DATA_COMMUNICATION_METADATA_H
#define DATA_COMMUNICATION_METADATA_H

#include "Def.h"
#include <QString>

namespace Data {

struct MsgInfo {
  MsgInfo(const QString &text, TtTextFormat::Type type, uint32_t time)
      : text(text), type(type), time(time) {}

  QString text;
  TtTextFormat::Type type;
  uint32_t time;
};

} // namespace Data

#endif // COMMUNICATION_METADATA_H

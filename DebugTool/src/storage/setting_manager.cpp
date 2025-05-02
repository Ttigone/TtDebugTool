#include "storage/setting_manager.h"

namespace Storage {

void SettingsManager::setSetting(const QString& key, const QJsonValue& value) {
  QMutexLocker locker(&mutex);
  settings[key] = value;
  saveSettings();
}

} // Storage

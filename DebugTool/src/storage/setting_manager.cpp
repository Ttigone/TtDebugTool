#include "storage/setting_manager.h"
#include <qmutex.h>

namespace Storage {

void SettingsManager::saveSettings() {
  QString targetFile = getConfigFilePath(file_path_);
  if (targetFile.isEmpty()) {
    qWarning() << "Invalid config file path";
    return;
  }

  // 读取文件的内容
  QJsonObject existingSettings;
  {
    // 读取现有文件内容并合并
    QFile readFile(targetFile);
    if (readFile.open(QIODevice::ReadOnly)) {
      QByteArray fileData = readFile.readAll();
      QJsonDocument existingDoc = QJsonDocument::fromJson(fileData);
      if (!existingDoc.isNull() && existingDoc.isObject()) {
        existingSettings = existingDoc.object();
      }
      readFile.close();
    }
  }

  // 将当前设置合并到现有内容中
  for (auto it = settings.begin(); it != settings.end(); ++it) {
    existingSettings[it.key()] = it.value(); // 同名键会被覆盖
  }

  QFile writeFile(targetFile);
  if (!writeFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    qWarning() << "Failed to open file for writing:" << writeFile.errorString();
    return;
  }
  if (writeFile.write(QJsonDocument(existingSettings).toJson()) == -1) {
    qWarning() << "Write error:" << writeFile.errorString();
  }
  writeFile.close();
  qDebug() << "write success";
}

void SettingsManager::loadSettings(const QString &filePath) {
  QMutexLocker locker(&mutex);

  QFile file(filePath);
  if (!file.open(QIODevice::ReadOnly)) {
    qWarning("Couldn't open load file.");
    return;
  }
  QByteArray fileData = file.readAll();
  file.close();

  QJsonDocument jsonDoc(QJsonDocument::fromJson(fileData));
  if (jsonDoc.isNull() || !jsonDoc.isObject()) {
    qWarning("Invalid JSON document.");
    return;
  }
  settings = jsonDoc.object();
}

QJsonObject SettingsManager::getHistorySettings() const {
  // 初始化的时候, 有调用这个
  QMutexLocker locker(&mutex);
  QString targetFile = getConfigFilePath(file_path_);
  if (targetFile.isEmpty()) {
    qWarning() << "Invalid config file path";
    return QJsonObject();
  }

  QJsonObject historySettings;
  {
    QFile readFile(targetFile);
    if (readFile.open(QIODevice::ReadOnly)) {
      QByteArray fileData = readFile.readAll();
      QJsonDocument existingDoc = QJsonDocument::fromJson(fileData);
      if (!existingDoc.isNull() && existingDoc.isObject()) {
        historySettings = existingDoc.object();
      }
      readFile.close();
    }
  }
  return historySettings;
}

void SettingsManager::setSetting(const QString &key, const QJsonValue &value) {
  QMutexLocker locker(&mutex);
  settings[key] = value;
  saveSettings();
}

QJsonValue SettingsManager::getSetting(const QString &key) const {
  // 需要先调用 loadSetting
  QMutexLocker locker(&mutex);
  if (settings.isEmpty()) {
    qWarning() << "No data to read";
  }
  return settings.value(key);
}

void SettingsManager::removeOneSetting(const QString &key) {
  // 写读取全部的, 在删除, 在重新写入
  QMutexLocker locker(&mutex);

  QString targetFile = getConfigFilePath(file_path_);
  if (targetFile.isEmpty()) {
    qWarning() << "Invalid config file path";
    return;
  }

  // 读取文件的内容
  QJsonObject existingSettings;
  {
    // 读取现有文件内容并合并
    QFile readFile(targetFile);
    if (readFile.open(QIODevice::ReadOnly)) {
      QByteArray fileData = readFile.readAll();
      QJsonDocument existingDoc = QJsonDocument::fromJson(fileData);
      if (!existingDoc.isNull() && existingDoc.isObject()) {
        existingSettings = existingDoc.object();
      }
      readFile.close();
    }
  }

  // 没找到
  if (!existingSettings.contains(key)) {
    // qWarning() << "Setting not found:" << key;
    // 直接 not found
    qDebug() << "Setting not found:" << key;
  }

  // 删除 key
  existingSettings.remove(key);

  QFile writeFile(targetFile);
  if (!writeFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    qWarning() << "Failed to open file for writing:" << writeFile.errorString();
    return;
  }
  // 重新写回去
  if (writeFile.write(QJsonDocument(existingSettings).toJson()) == -1) {
    qWarning() << "Write error:" << writeFile.errorString();
  }
  writeFile.close();
}

QString SettingsManager::getConfigFilePath(const QString &filename) const {
  // 获取当前可执行文件的路径
  QString appDirPath = QCoreApplication::applicationDirPath();
  // 创建 configs 目录路径
  QString configDirPath = appDirPath + "/configs";
  // 确保 configs 目录存在
  QDir dir;
  if (!dir.exists(configDirPath)) {
    if (!dir.mkpath(configDirPath)) {
      qWarning() << "Failed to create configs directory.";
      return QString();
    }
  }
  // 设置配置文件路径
  QString configFilePath = configDirPath + "/" + filename;
  return configFilePath;
}

} // namespace Storage

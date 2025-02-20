#ifndef STORAGE_SETTING_MANAGER_H
#define STORAGE_SETTING_MANAGER_H

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutex>
#include <QMutexLocker>

namespace Storage {

class SettingsManager {
 public:
  static SettingsManager& instance() {
    static SettingsManager instance;
    return instance;
  }

  void setTargetStoreFile(const QString& filePath) { file_path_ = filePath; }

  void saveSettings() {
    QMutexLocker locker(&mutex);
    QJsonDocument jsonDoc(settings);
    auto targetFile = getConfigFilePath(file_path_);
    QFile file(targetFile);
    if (!file.open(QIODevice::WriteOnly)) {
      qWarning("Couldn't open save file.");
      return;
    }
    file.write(jsonDoc.toJson());
    file.close();
  }

  void loadSettings(const QString& filePath) {
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

  QJsonObject getSettings() const {
    QMutexLocker locker(&mutex);
    return settings;
  }

  void setSetting(const QString& key, const QJsonValue& value) {
    QMutexLocker locker(&mutex);
    settings[key] = value;
    //saveSettings();
  }

  QJsonValue getSetting(const QString& key) const {
    QMutexLocker locker(&mutex);
    return settings.value(key);
  }

 private:
  SettingsManager() {}
  ~SettingsManager() {}
  SettingsManager(const SettingsManager&) = delete;
  SettingsManager& operator=(const SettingsManager&) = delete;

  QString getConfigFilePath(const QString& filename) {
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

  QString file_path_;
  mutable QMutex mutex;
  QJsonObject settings;
};

}  // namespace Storage

#endif  // STORAGE_SETTING_MANAGER_H

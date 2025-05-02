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
    // 不得再加锁了, 否则死锁
    QString targetFile = getConfigFilePath(file_path_);
    if (targetFile.isEmpty()) {
      qWarning() << "Invalid config file path";
      return;
    }

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
      existingSettings[it.key()] = it.value();  // 同名键会被覆盖
    }

    QFile writeFile(targetFile);
    if (!writeFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
      qWarning() << "Failed to open file for writing:"
                 << writeFile.errorString();
      return;
    }
    if (writeFile.write(QJsonDocument(existingSettings).toJson()) == -1) {
      qWarning() << "Write error:" << writeFile.errorString();
    }
    writeFile.close();
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

  QJsonObject getHistorySettings() const {
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

  void setSetting(const QString& key, const QJsonValue& value);

  QJsonValue getSetting(const QString& key) const {
    QMutexLocker locker(&mutex);
    return settings.value(key);
  }

 private:
  SettingsManager() {}
  ~SettingsManager() {}
  SettingsManager(const SettingsManager&) = delete;
  SettingsManager& operator=(const SettingsManager&) = delete;

  QString getConfigFilePath(const QString& filename) const {
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

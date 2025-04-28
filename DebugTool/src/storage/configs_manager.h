#ifndef STORAGE_CONFIGS_MANAGER_H
#define STORAGE_CONFIGS_MANAGER_H

#include <QMutex>
#include <QSettings>

namespace Storage {

class TtConfigsManager {
 public:
  static TtConfigsManager& instance() {
    static TtConfigsManager instance;
    return instance;
  }

  void setTargetStoreFile(const QString& filePath) { file_path_ = filePath; }

  void setConfigVaule(QAnyStringView key, const QVariant& value) {
    QSettings settings(getConfigFilePath(file_path_), QSettings::IniFormat);
    settings.setValue(key, value);
  }

  QVariant getConfigVaule(QAnyStringView key, const QVariant& defaultValue) {
    qDebug() << getConfigFilePath(file_path_);
    // qDebug() << getConfigFilePath(file_path_);

    QSettings settings(getConfigFilePath(file_path_), QSettings::IniFormat);
    // if (settings.)

    QVariant val = settings.value(key, defaultValue);
    // 如果文件刚好不存在，或者读取到的是 defaultValue，都把它写回去，保证文件被创建
    return val;
    // return settings.value(key, defaultValue);
  }

  void saveConfigs() {
    QMutexLocker locker(&mutex);
    // QSettings()
    // QJsonDocument jsonDoc(settings);
    auto targetFile = getConfigFilePath(file_path_);
    QFile file(targetFile);
    if (!file.open(QIODevice::WriteOnly)) {
      qWarning("Couldn't open save file.");
      return;
    }
    // file.write(jsonDoc.toJson());
    // file.close();
  }

 private:
  TtConfigsManager() {}
  ~TtConfigsManager() {}
  TtConfigsManager(const TtConfigsManager&) = delete;
  TtConfigsManager& operator=(const TtConfigsManager&) = delete;

  QString getConfigFilePath(const QString& filename) {
    // 获取当前可执行文件的路径
    QString appDirPath = QCoreApplication::applicationDirPath();
    QString configPath = appDirPath + filename;
    // 创建文件
    QFile file(configPath);
    if (!file.exists()) {
      if (file.open(QIODevice::WriteOnly)) {
        // 文件已创建
        file.close();
        qDebug() << "文件已创建:" << configPath;
      }
    }
    return configPath;
  }

  QString file_path_;
  mutable QMutex mutex;
  // QSettings settings;
};

}  // namespace Storage

#endif  // CONFIGS_MANAGER_H

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
  static SettingsManager &instance() {
    static SettingsManager instance;
    return instance;
  }

  ///
  /// @brief setTargetStoreFile
  /// @param filePath
  /// 设置目标文件
  void setTargetStoreFile(const QString &filePath) { file_path_ = filePath; }

  ///
  /// @brief saveSettings
  /// 保存配置到本地中
  void saveSettings();

  ///
  /// @brief loadSettings
  /// @param filePath
  /// 加载配置到 settings 中
  void loadSettings(const QString &filePath);

  ///
  /// @brief getHistorySettings
  /// @return
  /// 获取本地文件的内容
  QJsonObject getHistorySettings() const;

  ///
  /// @brief setSetting
  /// @param key
  /// @param value
  /// 写入单个配置, 相同则覆盖
  void setSetting(const QString &key, const QJsonValue &value);

  ///
  /// @brief getSetting
  /// @param key
  /// @return
  /// 读取单个配置
  QJsonValue getSetting(const QString &key) const;

  ///
  /// @brief removeOneSetting
  /// @param key
  /// 删除单个配置
  void removeOneSetting(const QString &key);

private:
  SettingsManager() {}
  ~SettingsManager() {}
  SettingsManager(const SettingsManager &) = delete;
  SettingsManager &operator=(const SettingsManager &) = delete;

  ///
  /// @brief getConfigFilePath
  /// @param filename
  /// @return
  /// 获取配置文件路径
  QString getConfigFilePath(const QString &filename) const;

  QString file_path_;
  mutable QMutex mutex;
  QJsonObject settings;
};

} // namespace Storage

#endif // STORAGE_SETTING_MANAGER_H

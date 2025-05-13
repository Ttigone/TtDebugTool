#ifndef STORAGE_SETTING_MANAGER_H
#define STORAGE_SETTING_MANAGER_H

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutex>
#include <QMutexLocker>

QT_BEGIN_NAMESPACE
class QTimer;
QT_END_NAMESPACE

namespace Storage {

class SettingsManager : public QObject {
  Q_OBJECT
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

  // 有锁
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

  void setMultipleSettings(const QJsonObject &settingsToAdd);

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

  // 设置自动保存延迟时间
  void setSaveDelay(int milliseconds);

  ///
  /// @brief forceSave
  /// 关闭时强制保存
  void forceSave();
private slots:
  ///
  /// @brief actualSaveSettings
  /// 实际的保存操作
  void actualSaveSettings();

private:
  SettingsManager() : QObject(nullptr) { initTimer(); }
  ~SettingsManager() {}
  SettingsManager(const SettingsManager &) = delete;
  SettingsManager &operator=(const SettingsManager &) = delete;

  ///
  /// @brief getConfigFilePath
  /// @param filename
  /// @return
  /// 获取配置文件路径
  QString getConfigFilePath(const QString &filename) const;

  void initTimer();

  ///
  /// @brief scheduleSave
  /// 延时保存
  void scheduleSave();

  void loadSettingsIfNeeded();

  // 备份文件
  void createBackUp();

  bool isFileValid(const QString &filePath);

  // 尝试从备份恢复
  bool recoverFromBackup();

  QString file_path_;
  mutable QMutex mutex;
  QJsonObject settings;
  bool dirty_{false};
  QTimer *save_timer_{nullptr};
  int save_delay_ms_{1000};
};

} // namespace Storage

#endif // STORAGE_SETTING_MANAGER_H

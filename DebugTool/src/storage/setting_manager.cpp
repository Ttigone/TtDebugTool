#include "storage/setting_manager.h"
#include <QTimer>
#include <qjsonobject.h>
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
  // QMutexLocker locker(&mutex);

  // QFile file(filePath);
  // if (!file.open(QIODevice::ReadOnly)) {
  //   qWarning("Couldn't open load file.");
  //   return;
  // }
  // QByteArray fileData = file.readAll();
  // file.close();

  // QJsonDocument jsonDoc(QJsonDocument::fromJson(fileData));
  // if (jsonDoc.isNull() || !jsonDoc.isObject()) {
  //   qWarning("Invalid JSON document.");
  //   return;
  // }
  // settings = jsonDoc.object();

  QMutexLocker locker(&mutex);

  // 不是有效的
  if (!isFileValid(filePath)) {
    qWarning() << "File is not valid:" << filePath;
    // 尝试从备份恢复
    if (recoverFromBackup()) {
      qDebug() << "Successfully recovered from backup";
    } else {
      qWarning("Backup recovery failed, starting with empty configuration");
      settings = QJsonObject(); // 使用空配置
      return;
    }
  }
  // 是有效的, 直接读取
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
  // 配置信息保存到 settings 中
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

  if (settings.contains(key) && settings[key] == value) {
    // 值没有变化, 不需要保存
    return;
  }
  settings[key] = value;
  // saveSettings();
  // 延时保存
  dirty_ = true;
  if (!save_timer_->isActive()) {
    save_timer_->start(save_delay_ms_);
  }
}

void SettingsManager::setMultipleSettings(const QJsonObject &settingsToAdd) {
  QMutexLocker locker(&mutex);
  bool changed = false;

  // 合并多个设置，只标记一次为脏
  for (auto it = settingsToAdd.constBegin(); it != settingsToAdd.constEnd();
       ++it) {
    if (!settings.contains(it.key()) || settings[it.key()] != it.value()) {
      settings[it.key()] = it.value();
      changed = true;
    }
  }

  if (changed) {
    dirty_ = true;
    if (!save_timer_->isActive()) {
      save_timer_->start(save_delay_ms_);
    }
  }
}

QJsonValue SettingsManager::getSetting(const QString &key) const {
  // // 需要先调用 loadSetting
  // QMutexLocker locker(&mutex);
  // if (settings.isEmpty()) {
  //   qWarning() << "No data to read";
  // }
  // return settings.value(key);
  QMutexLocker locker(&mutex);
  // 内存中存在, 直接返回内存中的值, 内存中的值, 应该时刻保持最新设置的值
  if (settings.contains(key)) {
    return settings.value(key);
  }
  // 如果内存中没有, 则从文件加载全部设置
  if (settings.isEmpty()) {
    // 加载配置到 settings 中
    const_cast<SettingsManager *>(this)->loadSettingsIfNeeded();
  }
  return settings.value(key);
}

void SettingsManager::removeOneSetting(const QString &key) {
  // 写读取全部的, 在删除, 在重新写入
  QMutexLocker locker(&mutex);

  if (settings.contains(key)) {
    settings.remove(key);
    dirty_ = true;

    if (!save_timer_->isActive()) {
      save_timer_->start(save_delay_ms_);
    }
  }

  // QString targetFile = getConfigFilePath(file_path_);
  // if (targetFile.isEmpty()) {
  //   qWarning() << "Invalid config file path";
  //   return;
  // }

  // // 读取文件的内容
  // QJsonObject existingSettings;
  // {
  //   // 读取现有文件内容并合并
  //   QFile readFile(targetFile);
  //   if (readFile.open(QIODevice::ReadOnly)) {
  //     QByteArray fileData = readFile.readAll();
  //     QJsonDocument existingDoc = QJsonDocument::fromJson(fileData);
  //     if (!existingDoc.isNull() && existingDoc.isObject()) {
  //       existingSettings = existingDoc.object();
  //     }
  //     readFile.close();
  //   }
  // }

  // // 没找到
  // if (!existingSettings.contains(key)) {
  //   // qWarning() << "Setting not found:" << key;
  //   // 直接 not found
  //   qDebug() << "Setting not found:" << key;
  // }

  // // 删除 key
  // existingSettings.remove(key);

  // QFile writeFile(targetFile);
  // if (!writeFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
  //   qWarning() << "Failed to open file for writing:" <<
  //   writeFile.errorString(); return;
  // }
  // // 重新写回去
  // if (writeFile.write(QJsonDocument(existingSettings).toJson()) == -1) {
  //   qWarning() << "Write error:" << writeFile.errorString();
  // }
  // writeFile.close();
}

void SettingsManager::setSaveDelay(int milliseconds) {
  save_delay_ms_ = milliseconds;
}

void SettingsManager::forceSave() {
  if (dirty_) {
    actualSaveSettings();
  }
}

void SettingsManager::actualSaveSettings() {
  QString targetFile = getConfigFilePath(file_path_);
  if (targetFile.isEmpty()) {
    qWarning() << "Invalid config file path";
    return;
  }
  // 创建备份文件
  createBackUp();

  // 准备新数据(与旧的数据合并)
  QJsonObject existingSettings;
  {
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
  // 合并内存中的设置
  for (auto it = settings.begin(); it != settings.end(); ++it) {
    existingSettings[it.key()] = it.value(); // 同名键会被覆盖
  }
  // 使用临时文件进行写入
  QString tempFile = targetFile + ".temp";
  QFile writeFile(tempFile);
  if (!writeFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    qWarning() << "Failed to open temp file for writing:"
               << writeFile.errorString();
    return;
  }
  // 写入数据
  if (writeFile.write(QJsonDocument(existingSettings).toJson()) == -1) {
    qWarning() << "Write error:" << writeFile.errorString();
    writeFile.close();
    QFile::remove(tempFile);
    return;
  }
  // 确保数据写入磁盘
  writeFile.flush();
  writeFile.close();

  // 原子替换旧文件
  if (QFile::exists(targetFile)) {
    if (!QFile::remove(targetFile)) {
      qWarning() << "Failed to remove old config file";
      QFile::remove(tempFile);
      return;
    }
  }

  if (!QFile::rename(tempFile, targetFile)) {
    qWarning() << "Failed to rename temp file to config file";
    return;
  }

  qDebug() << "Settings successfully saved";
  dirty_ = false;
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

void SettingsManager::initTimer() {
  save_timer_ = new QTimer(this);
  save_timer_->setSingleShot(true);
  connect(save_timer_, &QTimer::timeout, this,
          &SettingsManager::actualSaveSettings);
}

void SettingsManager::scheduleSave() {
  dirty_ = true;
  if (!save_timer_->isActive()) {
    save_timer_->start(save_delay_ms_);
  }
}

void SettingsManager::loadSettingsIfNeeded() {
  if (settings.isEmpty()) {
    QString targetFile = getConfigFilePath(file_path_);
    if (!targetFile.isEmpty()) {
      loadSettings(targetFile);
    }
  }
}

void SettingsManager::createBackUp() {
  QString targetFile = getConfigFilePath(file_path_);
  QString backupFile = targetFile + ".bak";
  QFile::remove(backupFile);           // 删除旧备份
  QFile::copy(targetFile, backupFile); // 创建新备份
}

bool SettingsManager::isFileValid(const QString &filePath) {
  QFile file(filePath);
  if (!file.open(QIODevice::ReadOnly)) {
    return false;
  }
  // 读取全部内容
  QByteArray data = file.readAll();
  file.close();
  // 解析
  QJsonParseError error;
  QJsonDocument doc = QJsonDocument::fromJson(data, &error);
  return error.error == QJsonParseError::NoError && doc.isObject();
}

bool SettingsManager::recoverFromBackup() {
  // 读取备份文件
  QString targetFile = getConfigFilePath(file_path_);
  QString backupFile = targetFile + ".bak";
  if (!QFile::exists(backupFile)) {
    // 备份文件不存在
    return false;
  }
  // 移除原有的备份文件
  QFile::remove(targetFile);
  // 复制备份文件到目标文件
  return QFile::copy(backupFile, targetFile);
}

} // namespace Storage

#include "lang/translation_manager.h"

#include <QTranslator>

namespace Lang {

// bool TtTranslationManager::setLanguage(const QString& langCode) {
//   // QString qmFile = QString(":/translations/myapp_%1.qm").arg(langCode);
//   QString qmFile = langCode;
//   if (translator_.load(qmFile)) {
//     qApp->installTranslator(&translator_);
//     current_lang_code_ = langCode;
//     current_qm_file_ = qmFile;
//     emit languageChanged(langCode);
//     return true;
//   }
//   return false;
// }

bool TtTranslationManager::setLanguage(const QString& prefixes,
                                       const QString& langCode) {
  // QString qmFile = QString(":/translations/myapp_%1.qm").arg(langCode);
  QString qmFile = prefixes + langCode;
  qDebug() << "load: " << qmFile;
  if (translator_.load(qmFile)) {
    qDebug() << "install success";
    qApp->installTranslator(&translator_);
    current_lang_code_ = langCode;
    current_qm_file_ = qmFile;
    emit languageChanged(langCode);
    return true;
  }
  return false;
}

QString TtTranslationManager::currentLanguage() const {
  return current_lang_code_;
}

QString TtTranslationManager::currentQmFile() const {
  return current_qm_file_;
}

}  // namespace Lang

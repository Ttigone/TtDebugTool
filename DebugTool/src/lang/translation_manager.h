#ifndef LANG_TRANSLATION_MANAGER_H
#define LANG_TRANSLATION_MANAGER_H

#include <QTranslator>

namespace Lang {

class TtTranslationManager : public QObject {
  Q_OBJECT
 public:
  static TtTranslationManager& instance() {
    static TtTranslationManager manager;
    return manager;
  }

  // bool setLanguage(const QString& langCode);
  bool setLanguage(const QString& prefixes, const QString& langCode);

  QString currentLanguage() const;
  QString currentQmFile() const;

 signals:
  void languageChanged(const QString& langCode);

 private:
  TtTranslationManager() {}
  ~TtTranslationManager() { qDebug() << "delete"; }

  QTranslator translator_;
  QString current_lang_code_;
  QString current_qm_file_;
};

}  // namespace Lang

#endif  // LANG_TRANSLATION_MANAGER_H

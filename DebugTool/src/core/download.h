#ifndef CORE_DOWNLOAD_H
#define CORE_DOWNLOAD_H

QT_BEGIN_NAMESPACE
class QNetworkAccessManager;
class QNetworkReply;
QT_END_NAMESPACE

namespace Core {

class Downloader : public QObject {
  Q_OBJECT
 public:
  explicit Downloader(QObject* parent = nullptr);
  ~Downloader();

 public slots:
  void download(const QUrl& url, const QString& file);

 private slots:
  void onDownloadFinished(QNetworkReply* reply);

 signals:
  void errorOccurred(const QString& error);
  void available(bool);
  void running(bool);
  void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);

 private:
  void saveToDisk(QNetworkReply*);

  QNetworkAccessManager* manager_;
  QString save_file_;
};

} // namespace Core

#endif  // CORE_DOWNLOAD_H

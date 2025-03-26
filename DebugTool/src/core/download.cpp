#include "core/download.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>

namespace Core {

Downloader::Downloader(QObject* parent) : QObject(parent) {
  manager_ = new QNetworkAccessManager(this);
  connect(manager_, &QNetworkAccessManager::finished, this,
          &Downloader::onDownloadFinished);
}

Downloader::~Downloader() {}

void Downloader::download(const QUrl& url, const QString& file) {
  save_file_ = file;
  QNetworkRequest request(url);
  QNetworkReply* reply = manager_->get(request);
  connect(reply, &QNetworkReply::downloadProgress, this,
          &Downloader::downloadProgress);
  emit available(false);
  emit running(true);
}

void Downloader::onDownloadFinished(QNetworkReply* reply) {
  if (reply->error() != QNetworkReply::NoError) {
    emit errorOccurred(reply->errorString());
  } else {
    saveToDisk(reply);
  }
  reply->deleteLater();
  emit available(true);
  emit running(false);
}

void Downloader::saveToDisk(QNetworkReply* reply) {
  QFile f(save_file_);
  f.open(QIODevice::WriteOnly | QIODevice::Truncate);
  f.write(reply->readAll());
  f.close();
}

}  // namespace Core

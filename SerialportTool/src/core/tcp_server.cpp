#include "core/tcp_server.h"

#include <QRandomGenerator>

namespace Core {

TcpServer::TcpServer(QObject* parent) : QTcpServer(parent) {
  // 初始化 fortune 列表，存储不同的命运消息（字符串）
  fortunes_ << tr("你过着狗一样的生活。别上家具。") << tr("你必须考虑明天。")
            << tr("你会被一声巨响惊讶。") << tr("再过一个小时你会感到饿。")
            << tr("你可能有邮件。")
            << tr("你不能在不伤害永恒的情况下消磨时间。")
            << tr("计算机并不聪明。它们只是认为自己聪明。");
}

void TcpServer::incomingConnection(qintptr socket_descriptor) {
  // 随机选择一个命运信息
  QString fortune =
      fortunes_.at(QRandomGenerator::global()->bounded(fortunes_.size()));

  // 创建新的 TcpSocketThread 线程对象, 处理连接
  TcpSocketThread* thread =
      new TcpSocketThread(socket_descriptor, fortune, this);

  // 连接线程的 finished 信号和 deleteLater 槽, 线程完成 run 后自动删除对象
  connect(thread, &TcpSocketThread::finished, thread,
          &TcpSocketThread::deleteLater);

  // 启动线程
  thread->start();
}

TcpSocketThread::TcpSocketThread(qintptr socket_descriptor,
                                 const QString& fortune, QObject* parent)
    : QThread(parent), socket_descriptor_(socket_descriptor), text_(fortune) {
  // 初始化线程, 保存套接字描述符和要发送的文本信息
}

void TcpSocketThread::run() {
  // Tcp 套接字
  QTcpSocket tcpSocket;

  // 设置套接字描述符, 失败发出信号
  if (!tcpSocket.setSocketDescriptor(socket_descriptor_)) {
    emit error(tcpSocket.error());
    return;
  }

  // 存储发送数据的块
  QByteArray block;
  // 数据流对象, 只写
  QDataStream out(&block, QIODevice::WriteOnly);
  // 设置数据流版本
  out.setVersion(QDataStream::Qt_6_4);
  // 写入数据流
  out << text_;

  // 数据块写入套接字
  tcpSocket.write(block);
  // 断开与主机的连接
  tcpSocket.disconnectFromHost();
  // 等待断开连接
  tcpSocket.waitForDisconnected();
}

}  // namespace Core

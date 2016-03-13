#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <QApplication>
#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTextEdit>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QTime>
#include <QInputDialog>

class ChatServer : public QWidget
{
    Q_OBJECT

public:
    ChatServer(QWidget *_parent = 0);
    ~ChatServer();

public slots:
    virtual void slotNewConnection();
    void slotReadClient();

private slots:
    void deleteSocket();

private:
    void sentToClient(QTcpSocket *_pSocket, const QString &_str);

    QTcpServer *tcpServer;
    QTextEdit *txt;
    quint16 nextBlockSize;

    QVector <QTcpSocket *> clientSockets;
};

#endif // CHATSERVER_H

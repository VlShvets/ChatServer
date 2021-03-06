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
    void sendListClientsToClient(QTcpSocket *_tcpSocket, const QString &_listClients);
    QTcpSocket *socketOfClients(const QString &_clientName);
    void sendMessageToClient(const QString &_sender, const QString &_addressee, const QString &_message);

    QTcpServer *tcpServer;
    QTextEdit *txt;
    quint16 nextBlockSize;

    QVector <QTcpSocket *> clientSockets;
};

#endif // CHATSERVER_H

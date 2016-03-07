#include "chatserver.h"

ChatServer::ChatServer(int _nPort, QWidget *_parent)
    : QWidget(_parent), nextBlockSize(0)
{
    tcpServer = new QTcpServer(this);
    if(!tcpServer->listen(QHostAddress::Any, _nPort))
    {
        QMessageBox::critical(0, "Server Eroor", "Unable to start the server:" + tcpServer->errorString());
        tcpServer->close();
        return;
    }
    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(slotNewConnection()));

    txt = new QTextEdit;
    txt->setReadOnly(true);

    QVBoxLayout *vbxLayout = new QVBoxLayout;
    vbxLayout->addWidget(new QLabel("<H1>Server</H1>"));
    vbxLayout->addWidget(txt);
    setLayout(vbxLayout);
}

ChatServer::~ChatServer()
{

}

void ChatServer::slotNewConnection()
{
    QTcpSocket *clientSocket = tcpServer->nextPendingConnection();
    connect(clientSocket, SIGNAL(disconnected()), clientSocket, SLOT(deleteLater()));
    connect(clientSocket, SIGNAL(readyRead()), this, SLOT(slotReadClient()));
    sentToClient(clientSocket, "Server Response: Connected!");
}

void ChatServer::slotReadClient()
{
    QTcpSocket *clientSocket = (QTcpSocket*) sender();
    QDataStream in(clientSocket);
    in.setVersion(QDataStream::Qt_5_5);
    for(;;)
    {
        if(!nextBlockSize)
        {
            if(clientSocket->bytesAvailable() < sizeof(quint16))
                break;
            in >> nextBlockSize;
        }

        if(clientSocket->bytesAvailable() < nextBlockSize)
            break;

        QTime time;
        QString str;
        in >> time >> str;

        QString strMessage = time.toString() + " " + "Client has sent - " + str;
        txt->append(strMessage);

        nextBlockSize = 0;

        sentToClient(clientSocket, "Sent Response: Received \"" + str + "\"");
    }
}

void ChatServer::sentToClient(QTcpSocket *_pSocket, const QString &_str)
{
    QByteArray arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_5);
    out << quint16(0) << QTime::currentTime() << _str;

    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));

    _pSocket->write(arrBlock);
}

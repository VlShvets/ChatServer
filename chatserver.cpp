#include "chatserver.h"

ChatServer::ChatServer(QWidget *_parent)
    : QWidget(_parent), nextBlockSize(0)
{
    this->setWindowTitle(tr("Сервер"));

    tcpServer = new QTcpServer(this);

    bool bOk;
    quint16 nPort = QInputDialog::getInt(0, tr("Номер порта"), tr("Введите номер порта:"),
                                        2323, 0, 65535, 1, &bOk);
    if(!bOk)
        nPort = 2323;

    if(!tcpServer->listen(QHostAddress::Any, nPort))
    {
        QMessageBox::critical(0, tr("Ошибка Сервера"), tr("Невозможно запустить сервер:") + " " + tcpServer->errorString());
        tcpServer->close();
        return;
    }
    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(slotNewConnection()));

    txt = new QTextEdit;
    txt->setReadOnly(true);

    QVBoxLayout *vbxLayout = new QVBoxLayout;
    vbxLayout->addWidget(new QLabel(tr("<H1>Сервер</H1>")));
    vbxLayout->addWidget(txt);
    setLayout(vbxLayout);

    txt->append(tr("Порт") + " " + QString::number(nPort) + ". " + tr("Сервер запущен...") + "<br>");
}

ChatServer::~ChatServer()
{

}

void ChatServer::slotNewConnection()
{
    clientSockets.push_back(tcpServer->nextPendingConnection());

    connect(clientSockets.last(), SIGNAL(disconnected()), clientSockets.last(), SLOT(deleteLater()));
    connect(clientSockets.last(), SIGNAL(disconnected()), this, SLOT(deleteSocket()));
    connect(clientSockets.last(), SIGNAL(readyRead()), this, SLOT(slotReadClient()));
}

void ChatServer::slotReadClient()
{
    QTcpSocket *clientSocket = (QTcpSocket *) sender();
    QDataStream in(clientSocket);
    in.setVersion(QDataStream::Qt_5_4);
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

        QString addressee;
        QTime time;
        QString str;
        in >> addressee >> time >> str;

        QString strMessage;
        if(clientSocket->objectName() == "")
        {
            clientSocket->setObjectName(addressee);
            strMessage = tr("Пользователь") + " <b>" + addressee + "</b> " +
                    tr("был подключен успешно. Всего пользователей в сети") + " " + QString::number(clientSockets.count());
            txt->append(time.toString() + " " + strMessage + "<br>");

            strMessage.clear();
            for(int i = 0; i < clientSockets.count(); ++i)
                strMessage += clientSockets.at(i)->objectName() + "0x00";
            for(int i = 0; i < clientSockets.count(); ++i)
                sentToClient(clientSockets.at(i)->objectName(), clientSockets.at(i), strMessage);
        }
        else
        {
            strMessage = "<b>" + clientSocket->objectName() + "</b><br>" + str + "<br>";
            txt->append(time.toString() + " " + strMessage);
        }

        nextBlockSize = 0;
    }
}

void ChatServer::deleteSocket()
{
    QTcpSocket *clientSocket = (QTcpSocket *) sender();
    clientSockets.removeAll(clientSocket);

    QString strMessage = tr("Пользователь") + " <b>" + clientSocket->objectName() + "</b> " +
            tr("отключился. Всего пользователей в сети") + " " + QString::number(clientSockets.count());
    txt->append(QTime::currentTime().toString() + " " + strMessage + "<br>");
}

void ChatServer::sentToClient(const QString &_sender, QTcpSocket *_pSocket, const QString &_message)
{
    QByteArray arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_4);
    out << quint16(0) << _sender << QTime::currentTime() << _message;

    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));

    _pSocket->write(arrBlock);
}

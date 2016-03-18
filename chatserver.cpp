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

        bool initialization;
        in >> initialization;

        if(initialization)
        {
            QString clientName;
            in >> clientName;
            clientSocket->setObjectName(clientName);
            txt->append(QTime::currentTime().toString() + " " + tr("Пользователь") + " <b>" + clientName + "</b> " +
                        tr("был подключен успешно. Всего пользователей в сети") + " " +
                        QString::number(clientSockets.count()) + "<br>");

            /// Формирование списка клиентов
            QString listClients;
            for(int i = 0; i < clientSockets.count(); ++i)
                listClients += clientSockets.at(i)->objectName() + "0x00";

            /// Рассылка списка клиентов
            for(int i = 0; i < clientSockets.count(); ++i)
                sendListClientsToClient(clientSockets.at(i), listClients);
        }
        else
        {
            QString str;
            in >> str;

            QStringList addresseeMessage = str.split("0x00", QString::SkipEmptyParts);
            QString addressee = addresseeMessage.first();
            QString message = addresseeMessage.last();

            txt->append(QTime::currentTime().toString() + " <b>" + clientSocket->objectName() + " " +
                        addressee + "</b><br>" + message + "<br>");
            sendMessageToClient(clientSocket->objectName(), addressee, message);
        }

        nextBlockSize = 0;
    }
}

void ChatServer::deleteSocket()
{
    QTcpSocket *clientSocket = (QTcpSocket *) sender();
    clientSockets.removeAll(clientSocket);
    txt->append(QTime::currentTime().toString() + " " + tr("Пользователь") + " <b>" + clientSocket->objectName() + "</b> " +
                tr("отключился. Всего пользователей в сети") + " " + QString::number(clientSockets.count()) + "<br>");
}

void ChatServer::sendListClientsToClient(QTcpSocket *_tcpSocket, const QString &_listClients)
{
    QByteArray arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_4);
    out << quint16(0) << true << _listClients;

    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));

    _tcpSocket->write(arrBlock);
}

QTcpSocket *ChatServer::socketOfClients(const QString &_clientName)
{
    for(int i = 0; i < clientSockets.count(); ++i)
    {
        if(clientSockets.at(i)->objectName() == _clientName)
            return clientSockets.at(i);
    }
    return NULL;
}

void ChatServer::sendMessageToClient(const QString &_sender, const QString &_addressee, const QString &_message)
{
    QString str = _sender + "0x00" + _message;

    QByteArray arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_4);
    out << quint16(0) << false << str;

    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));

    socketOfClients(_addressee)->write(arrBlock);
}

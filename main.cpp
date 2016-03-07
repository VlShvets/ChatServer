#include "chatserver.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    ChatServer server(2323);
    server.show();

    return a.exec();
}

#include "chatserver.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    ChatServer w(2323);
    w.show();

    return a.exec();
}

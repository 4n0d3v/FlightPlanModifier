#include "dialog.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Dialog w;
    if (w.error)
        return EXIT_FAILURE;
    w.show();
    return a.exec();
}

#include <QApplication>
#include "insertwindow.h"


int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("DataStorm");
    QApplication::setApplicationVersion("1.0");
    QApplication::setOrganizationName("DataStorm");
    QApplication::setWindowIcon(QIcon("/Users/maximecolliat/CLionProjects/DataStorm/assets/datastorm.png"));

//    HomeWindow window;
//    window.show();
    InsertWindow window;
    window.show();
    return QApplication::exec();
}
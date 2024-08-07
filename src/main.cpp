#include <QApplication>
#include "insertwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("DataStorm");
    QApplication::setApplicationVersion("1.0");
    QApplication::setOrganizationName("DataStorm");
    QApplication::setWindowIcon(QIcon(QCoreApplication::applicationDirPath() + "/assets/datastorm.png"));
    QApplication::setApplicationDisplayName("DataStorm");

    InsertWindow window;
    window.show();
    return QApplication::exec();
}
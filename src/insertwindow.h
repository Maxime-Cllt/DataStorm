#ifndef INSERTWINDOW_H
#define INSERTWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QSqlDatabase>
#include <fstream>
#include <string>

namespace Ui {
    class InsertWindow;
}

class InsertWindow : public QMainWindow {
Q_OBJECT

public:
    explicit InsertWindow(QWidget *parent = nullptr);

    ~InsertWindow() override;

    void addToolbar();

    void openFile();

    void insertFile();

    void clearTable();

    void addLog(const QString &message);

    void set_connected(bool connected);

    bool connect_to_db();

    void dropAndCreateTable();

    void alterTable();

    void loadCSV();

private:
    Ui::InsertWindow *ui;
    QString fileName;
    QPushButton *clearButton{};
    QPushButton *insertSqlButton{};
    QPushButton *openButton{};
    QSqlDatabase database;
    QStringList headers;
    char separator = ';';
};

#endif // INSERTWINDOW_H

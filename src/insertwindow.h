#ifndef INSERTWINDOW_H
#define INSERTWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QSqlDatabase>
#include <thread>
#include <fstream>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <atomic>

namespace Ui {
    class InsertWindow;
}

class InsertWindow : public QMainWindow {
Q_OBJECT

public:
    explicit InsertWindow(QWidget *parent = nullptr);

    ~InsertWindow() override;

    void addToolbar();

    void open_file();

    void insert_file();

    void clear_table();

    void addLog(const QString &message);

    void set_connected(bool connected);

    bool connect_to_db();

    void dropAndCreateTable();

    void alterTable();

private:
    Ui::InsertWindow *ui;
    QString fileName;
    QPushButton *clearButton{};
    QPushButton *insertSqlButton{};
    QPushButton *openButton{};
    QSqlDatabase database;
    std::queue<std::string> lineQueue;
    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<bool> done{false};
    QStringList headers;
    char separator = ';';
};

#endif // INSERTWINDOW_H

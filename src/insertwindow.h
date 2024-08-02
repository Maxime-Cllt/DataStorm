#ifndef INSERTWINDOW_H
#define INSERTWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QSqlDatabase>


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

private:
    Ui::InsertWindow *ui;
    QString fileName;
    QPushButton *clearButton{};
    QPushButton *insertSqlButton{};
    QPushButton *openButton{};
    QSqlDatabase database;
};

#endif // INSERTWINDOW_H

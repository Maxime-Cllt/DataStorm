#ifndef INSERTWINDOW_H
#define INSERTWINDOW_H

#include <QMainWindow>
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

    void addMenuBar();

    void openFile();

    void insertFile();

    void clearTable();

    void addLog(const QString &message);

    void setConnected(bool connected);

    bool connectToDb();

    void dropAndCreateTable();

    void alterTable(const QMap<QString, int> &maxLenghtColumns);

    void loadCSV();

    void saveConfig();

    void loadConfig();

private:
    Ui::InsertWindow *ui;
    QString fileName;
    QPushButton *clearButton{};
    QPushButton *insertSqlButton{};
    QPushButton *openButton{};
    QSqlDatabase database;
    QStringList headers;
    char separator = ';';
    QString connectionType;
};

#endif // INSERTWINDOW_H

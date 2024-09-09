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

    void clearUi();

    void addLog(const QString &message);

    void setConnected(bool connected);

    bool connectToDb();

    void saveConfig();

    void loadConfig();

    [[nodiscard]] QSqlDatabase *getDatabase() const;

    [[nodiscard]] Ui::InsertWindow *getUi() const;

    [[nodiscard]] const QString *getFileName() const;

private:
    Ui::InsertWindow *ui;
    QString fileName;
    QPushButton *clearButton{};
    QPushButton *insertSqlButton{};
    QPushButton *openButton{};
    QSqlDatabase database;
    QString connectionType;
};

#endif // INSERTWINDOW_H

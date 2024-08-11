#ifndef DATABASECONFIG_H
#define DATABASECONFIG_H

#include <QWidget>

namespace Ui {
class DatabaseConfig;
}

class DatabaseConfig : public QWidget
{
    Q_OBJECT

public:
    explicit DatabaseConfig(QWidget *parent = nullptr);
    ~DatabaseConfig();

private:
    Ui::DatabaseConfig *ui;
};

#endif // DATABASECONFIG_H

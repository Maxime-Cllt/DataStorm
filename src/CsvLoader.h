#include <QString>
#include <QList>
#include <QSqlDatabase>
#include <ui_insertwindow.h>
#include "insertwindow.h"

#ifndef DATASTORM_CSVLOADER_H
#define DATASTORM_CSVLOADER_H

class CsvLoader {

public:
    explicit CsvLoader(InsertWindow &insertWindow);

    ~CsvLoader();

    void loadCSV();

    void dropAndCreateTable();

    void optimiseTable(const QMap<QString, int> &maxLenghtColumns);

    QString createTempTableSQL() const;

    QString dropTableSQL(const QString &tableName) const;

private:
    QStringList headers;
    char separator = ';';
    QString connectionType;
    InsertWindow *insertWindow;
};

#endif //DATASTORM_CSVLOADER_H

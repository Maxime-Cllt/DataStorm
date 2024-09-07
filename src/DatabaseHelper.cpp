
#include "DatabaseHelper.h"
#include <QMap>
#include <QString>

const QString DatabaseHelper::QSQLITE = "QSQLITE";
const QString DatabaseHelper::QMYSQL = "QMYSQL";
const QString DatabaseHelper::QPSQL = "QPSQL";
const QString DatabaseHelper::QMARIADB = "QMARIADB";
const QString DatabaseHelper::QODBC = "QODBC";


QString DatabaseHelper::getDropTableSQL(const QString &connectionType, const QString &tableName) {
    const QMap<QString, QString> &dropTableSQL = {
            {DatabaseHelper::QSQLITE,  "DROP TABLE IF EXISTS "},
            {DatabaseHelper::QMARIADB, "DROP TABLE IF EXISTS "},
            {DatabaseHelper::QMYSQL,   "DROP TABLE IF EXISTS "},
            {DatabaseHelper::QODBC,    "DROP TABLE "},
            {DatabaseHelper::QPSQL,    "DROP TABLE IF EXISTS "}
    };
    return dropTableSQL[connectionType] % tableName;
}

QString DatabaseHelper::getCreateTempTableSQL(const QString &connectionType, const QString &tableName,
                                              const QStringList &columnNames) {
    QString sql;
    const int &headersSize = static_cast<int>(columnNames.size());
    const QMap<QString, QString> &sqlTemporaryTables = {
            {"QSQLITE",  "CREATE TEMP TABLE "},
            {"QMARIADB", "CREATE TEMPORARY TABLE "},
            {"QMYSQL",   "CREATE TEMPORARY TABLE "},
            {"QODBC",    "CREATE TEMPORARY TABLE "},
            {"QPSQL",    "CREATE TEMPORARY TABLE "}
    };
    sql = sqlTemporaryTables[connectionType] % tableName % "_temp (";
    for (unsigned int i = 0; i < headersSize; ++i) {
        sql += columnNames[i] % " TEXT";
        if (i < headersSize - 1) sql += ", ";
    }
    sql += ")";
    return sql;
}

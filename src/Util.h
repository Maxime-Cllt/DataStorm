//
// Created by Maxime Colliat on 02/08/2024.
//

#ifndef DATASTORM_UTIL_H
#define DATASTORM_UTIL_H

#include <QString>
#include <QFile>
#include <QIODeviceBase>
#include <QTextStream>
#include <QMessageBox>
#include <QThread>


unsigned int countLinesInFile(const QString &fileName) {
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(nullptr, "Erreur", "Impossible d'ouvrir le fichier : " + file.errorString());
        return -1;
    }

    QTextStream in(&file);
    unsigned int lineCount = 0;
    while (!in.atEnd()) {
        in.readLine();
        ++lineCount;
    }

    file.close();
    return lineCount;
}

QStringList normalizeColumnNames(const QStringList &columnNames) {
    QStringList normalizedColumnNames;
    for (const QString &columnName: columnNames) {
        QString normalizedColumnName = columnName;
        normalizedColumnName = normalizedColumnName.toLower();
        normalizedColumnName = normalizedColumnName.replace(" ", "_");
        normalizedColumnNames.append(normalizedColumnName);
    }
    return normalizedColumnNames;
}

QString insertData(const QString &tableName, const QStringList &columns, const QStringList &values) {
    QString sql = "INSERT INTO " + tableName + " (";
    for (int j = 0; j < columns.size(); j++) {
        sql += columns[j];
        if (j < columns.size() - 1) sql += ", ";
    }
    sql += ") VALUES (";
    for (int j = 0; j < values.size(); j++) {
        sql += "'" + values[j] + "'";
        if (j < values.size() - 1) sql += ", ";
    }
    sql += ")";
    return sql;
}

#endif //DATASTORM_UTIL_H

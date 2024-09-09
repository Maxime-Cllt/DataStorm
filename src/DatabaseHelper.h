//
// Created by Maxime Colliat on 06/09/2024.
//

#ifndef DATASTORM_DATABASEHELPER_H
#define DATASTORM_DATABASEHELPER_H

#include <QString>

class DatabaseHelper {
public:

    /**
     * @brief The connection types for the database connection
     */
    static const QString QSQLITE;
    static const QString QMYSQL;
    static const QString QPSQL;
    static const QString QMARIADB;
    static const QString QODBC;

    /**
     * Crée la requête SQL pour supprimer une table de la base de données si elle existe
     * @return La requête SQL
     */
    static QString getDropTableSQL(const QString &connectionType, const QString &tableName);


/**
 * Crée la requête SQL pour créer une table temporaire
 * @return La requête SQL
 */
    static QString
    getCreateTempTableSQL(const QString &connectionType, const QString &tableName, const QStringList &columnNames);
};

#endif //DATASTORM_DATABASEHELPER_H

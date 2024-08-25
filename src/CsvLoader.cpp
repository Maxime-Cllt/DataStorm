#include "CsvLoader.h"

#include "datastorm.h"
#include <iostream>
#include <fstream>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>

CsvLoader::CsvLoader(InsertWindow &insertWindow) : insertWindow(&insertWindow) {
    this->connectionType = insertWindow.getDatabase()->driverName();
}

CsvLoader::~CsvLoader() = default;

/**
 * Charge le fichier CSV et insère les données dans la base de données
 */
void CsvLoader::loadCSV() {

    std::ifstream file(this->insertWindow->getFileName()->toStdString());
    if (!file.is_open()) {
        this->insertWindow->addLog("Impossible d'ouvrir le fichier");
        QMessageBox::critical(this->insertWindow, "Erreur", "Impossible d'ouvrir le fichier");
        return;
    }

    const auto start = std::chrono::high_resolution_clock::now();

    std::string line;
    std::getline(file, line);
    this->separator = line.find(';') != std::string::npos ? ';' : ',';
    this->headers = normalizeColumnNames(QString::fromStdString(line).split(this->separator));
    this->insertWindow->addLog("Colonnes : " + this->headers.join(", ") + " (séparateur : " + this->separator + ")" +
                               " (nombre de colonnes : " + QString::number(this->headers.size()) + ")");
    this->insertWindow->addLog("Insertion des données...");

    this->insertWindow->getDatabase()->transaction();

    if (!this->insertWindow->getUi()->methodeBox->isChecked()) this->dropAndCreateTable();

    this->insertWindow->addLog("Temps création : " +
                               QString::number(std::chrono::duration_cast<std::chrono::milliseconds>(
                                       std::chrono::high_resolution_clock::now() - start).count()) + " ms");

    QSqlQuery query(*this->insertWindow->getDatabase());
    QString insertSQL = "INSERT INTO " % this->insertWindow->getUi()->tableName->text().trimmed() % "_temp (";
    insertSQL += this->headers.join(", ") % ") VALUES (";
    insertSQL += QString("?, ").repeated(this->headers.size());
    insertSQL.chop(2);
    insertSQL += ")";

    if (!query.prepare(insertSQL)) {
        this->insertWindow->addLog("Erreur lors de la préparation de l'insertion : " % query.lastError().text());
        QMessageBox::critical(nullptr, "Erreur", "Impossible de préparer l'insertion : " % query.lastError().text());
        return;
    }

    const short batchSize = 1000;
    unsigned int rowCount = 0;

    QMap<QString, int> maxLengths;
    for (const auto &header: this->headers) {
        maxLengths[header] = 0;
    }
    QStringList lineValues;

    while (std::getline(file, line)) {
        lineValues = QString::fromStdString(line).split(this->separator, Qt::KeepEmptyParts);
        const int valuesSize = static_cast<int>(lineValues.size());
        for (int i = 0; i < valuesSize; ++i) {
            const QString &value = lineValues[i];
            const int length = (int) value.length();
            if (length > maxLengths[this->headers[i]]) {
                maxLengths[this->headers[i]] = length;
            }
            query.bindValue(i, value);
        }

        if (!query.exec()) {
            this->insertWindow->addLog("Erreur lors de l'insertion de la ligne : " + query.lastError().text());
        }

        if (++rowCount == batchSize) {
            this->insertWindow->getDatabase()->commit();
            this->insertWindow->getDatabase()->transaction();
//            QMetaObject::invokeMethod(this->ui->progressBar, "setValue", Qt::QueuedConnection,
//                                      Q_ARG(int, (int) ((rowCount / (double) this->headers.size()) * 100)));
        }
    }

    this->insertWindow->getDatabase()->commit();

    this->insertWindow->addLog("Temps insertion : " +
                               QString::number(std::chrono::duration_cast<std::chrono::milliseconds>(
                                       std::chrono::high_resolution_clock::now() - start).count()) + " ms");

    // Optimisation de la table pour réduire l'espace disque en modifiant la taille des colonnes
    this->optimiseTable(maxLengths);

    this->insertWindow->addLog("Temps d'exécution total: " +
                               QString::number(std::chrono::duration_cast<std::chrono::milliseconds>(
                                       std::chrono::high_resolution_clock::now() - start).count()) + " ms");

    file.close();
    query.clear();
    query.finish();
}

/**
 * Normalise les noms des colonnes
 * @param columns Les noms des colonnes
 * @return Les noms des colonnes normalisés
 */
void CsvLoader::optimiseTable(const QMap<QString, int> &maxLenghtColumns) {
    const QString &tableName = this->insertWindow->getUi()->tableName->text().trimmed();
    QStringList columnDefinitions;
    QSqlQuery query(*this->insertWindow->getDatabase());

    auto start = std::chrono::high_resolution_clock::now();

    for (const auto &header: this->headers) {
        const QString &columnDef = header % " VARCHAR(" % QString::number(maxLenghtColumns[header]) % ")";
        columnDefinitions.append(columnDef);
    }

    QString createTableSQL = "CREATE TABLE " % tableName % " (";
    for (const auto &columnDef: columnDefinitions) {
        createTableSQL += columnDef % ", ";
    }
    createTableSQL.chop(2);
    createTableSQL += ")";

    const QString copyDataSQL = QString("INSERT INTO %1 SELECT * FROM %1_temp").arg(tableName);
    const QString dropOldTableSQL = QString("DROP TABLE %1_temp").arg(tableName);

    if (!query.exec(createTableSQL)) {
        this->insertWindow->addLog("Erreur lors de la création de la nouvelle table : " % query.lastError().text());
        return;
    }

    this->insertWindow->addLog("Temps optimisation : " +
                               QString::number(std::chrono::duration_cast<std::chrono::milliseconds>(
                                       std::chrono::high_resolution_clock::now() - start).count()) + " ms");
    start = std::chrono::high_resolution_clock::now();

    if (!query.exec(copyDataSQL)) {
        this->insertWindow->addLog("Erreur lors de la copie des données : " % query.lastError().text());
        return;
    }

    this->insertWindow->addLog("Temps copie : " +
                               QString::number(std::chrono::duration_cast<std::chrono::milliseconds>(
                                       std::chrono::high_resolution_clock::now() - start).count()) + " ms");

    if (!query.exec(dropOldTableSQL)) {
        this->insertWindow->addLog("Erreur lors de la suppression de l'ancienne table : " % query.lastError().text());
        return;
    }

    query.clear();
    query.finish();
}

/**
 * Supprime la table si elle existe et la recrée avec les colonnes du CSV
 */
void CsvLoader::dropAndCreateTable() {
    QSqlQuery query(*this->insertWindow->getDatabase());
    const QString &tableName = this->insertWindow->getUi()->tableName->text().trimmed();

    if (!query.exec(this->dropTableSQL(tableName))) {
        this->insertWindow->addLog("Erreur lors de la suppression de la table : " % query.lastError().text());
        return;
    }

    if (!query.exec(this->createTempTableSQL())) {
        this->insertWindow->addLog("Erreur lors de la création de la table : " % query.lastError().text());
        QMessageBox::critical(nullptr, "Erreur", "Impossible de créer la table : " % query.lastError().text());
        return;
    }

    query.clear();
    query.finish();
}


/**
 * Crée la requête SQL pour créer une table temporaire
 * @return La requête SQL
 */
QString CsvLoader::createTempTableSQL() const {
    QString sql;
    const QString &tableName = this->insertWindow->getUi()->tableName->text().trimmed();
    const int &headersSize = static_cast<int>(this->headers.size());
    const QMap<QString, QString> &sqlTemporaryTables = {
            {"QSQLITE",  "CREATE TEMP TABLE "},
            {"QMARIADB", "CREATE TEMPORARY TABLE "},
            {"QMYSQL",   "CREATE TEMPORARY TABLE "},
            {"QODBC",    "CREATE TEMPORARY TABLE "},
            {"QPSQL",    "CREATE TEMPORARY TABLE "}
    };

    sql = sqlTemporaryTables[this->connectionType] % tableName % "_temp (";
    for (unsigned int i = 0; i < headersSize; ++i) {
        sql += this->headers[i] % " TEXT";
        if (i < headersSize - 1) sql += ", ";
    }
    sql += ")";
    return sql;
}

/**
 * Crée la requête SQL pour supprimer une table
 * @param tableName Le nom de la table
 * @return La requête SQL
 */
QString CsvLoader::dropTableSQL(const QString &tableName) const {
    const QMap<QString, QString> &dropTableSQL = {
            {"QSQLITE",  "DROP TABLE IF EXISTS "},
            {"QMARIADB", "DROP TABLE IF EXISTS "},
            {"QMYSQL",   "DROP TABLE IF EXISTS "},
            {"QODBC",    "DROP TABLE "},
            {"QPSQL",    "DROP TABLE IF EXISTS "}
    };
    return dropTableSQL[this->connectionType] % tableName;
}

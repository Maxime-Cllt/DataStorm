#include "insertwindow.h"
#include "ui_insertwindow.h"
#include "Util.h"
#include <QToolBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <iterator>
#include <QTextStream>
#include <QProgressBar>
#include <QThread>
#include <iostream>
#include <fstream>

InsertWindow::InsertWindow(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::InsertWindow) {
    this->ui->setupUi(this);

    this->addToolbar();

    // Bouton pour connecter une base de données
    this->ui->connectButton->setIcon(QIcon::fromTheme("network-connect"));
    this->ui->connectButton->setToolTip("Se connecter à une base de données");
    this->ui->connectButton->setStyleSheet(
            "QPushButton {background-color: #4CAF50; color: white; border: none; padding: 5px 10px; border-radius: 5px; font-weight: bold;}"
            "QPushButton:hover {background-color: #388E3C;}"
            "QPushButton:pressed {background-color: #2E7D32;}");

    // Bouton pour le statut de la connexion de la base de données
    this->ui->stateButton->setIcon(QIcon::fromTheme("network-disconnect"));
    this->ui->stateButton->setToolTip("Non connecté");
    this->ui->stateButton->setStyleSheet(
            "QPushButton {background-color: #FFC107; color: white; border: none; padding: 5px 10px; border-radius: 5px; font-weight: bold;}"
            "QPushButton:hover {background-color: #FFA000;}"
            "QPushButton:pressed {background-color: #FF8F00;}");
    this->ui->stateButton->setEnabled(false);
    this->ui->stateButton->setText("");

    // TextEdit pour afficher des messages de débogage
    this->ui->debugEdit->setReadOnly(true);
    this->ui->debugEdit->setAutoFormatting(QTextEdit::AutoAll);

    this->ui->progressBar->setMinimum(0);
    this->ui->progressBar->setMaximum(100);
    this->ui->progressBar->setValue(0);
    this->ui->progressBar->setVisible(false);
    this->ui->progressBar->setAlignment(Qt::AlignCenter);
    this->ui->progressBar->setStyleSheet(
            "QProgressBar {background-color: #f5f5f5; border: 1px solid #f5f5f5; border-radius: 5px;}"
            "QProgressBar::chunk {background-color: #4CAF50; border-radius: 5px;}"
            "QProgressBar::text {color: black; font-weight: bold;}"
            "QProgressBar::text::percentage {color: black; font-weight: bold;}"
            "QProgressBar::text::value {color: black; font-weight: bold;}"
            "QProgressBar::text::percentage::value {color: black; font-weight: bold;}");

    this->ui->methodeBox->setText("APPEND TO TABLE");
    this->ui->methodeBox->setStyleSheet(
            "QLineEdit {background-color: #f5f5f5; border: 1px solid #f5f5f5; border-radius: 5px; padding: 5px;}");
    this->ui->methodeBox->setEnabled(false);
    this->ui->methodeBox->setVisible(false);


    // SLOT connections
    connect(insertSqlButton, &QPushButton::clicked, this, &InsertWindow::insert_file);
    connect(clearButton, &QPushButton::clicked, this, &InsertWindow::clear_table);
    connect(openButton, &QPushButton::clicked, this, &InsertWindow::open_file);
    connect(this->ui->connectButton, &QPushButton::clicked, this, &InsertWindow::connect_to_db);
}

InsertWindow::~InsertWindow() {
    delete this->ui;
    delete clearButton;
    delete insertSqlButton;
    delete openButton;
    if (this->database.isOpen()) {
        this->database.close();
    }
}

void InsertWindow::open_file() {
    this->fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "",
                                                  tr("CSV Files (*.csv);;All Files (*)"));
    if (this->fileName.isEmpty()) return;

    QStringList parts = this->fileName.split("/");
    QFile file(this->fileName);
    this->ui->fileName->setAlignment(Qt::AlignCenter);
    this->ui->fileName->setText(
            "Fichier: " + parts[parts.size() - 1] + " (" + QString::number(file.size() / 1024) + " Ko)");

    this->ui->tableName->setText(get_name_for_table(this->fileName));
    this->ui->methodeBox->setEnabled(false);
    this->ui->methodeBox->setVisible(false);
    addLog("Fichier ouvert : " + this->fileName);
}

void InsertWindow::insert_file() {
    if (this->fileName.isEmpty()) {
        QMessageBox::warning(nullptr, "Fichier non ouvert", "Veuillez ouvrir un fichier avant de l'insérer");
        return;
    }
    if (!database.isOpen()) {
        QMessageBox::warning(nullptr, "Base de données non connectée",
                             "Veuillez vous connecter à une base de données avant d'insérer le fichier");
        return;
    }


    std::ifstream file(this->fileName.toStdString());
    if (!file.is_open()) {
        addLog("Impossible d'ouvrir le fichier");
        QMessageBox::critical(nullptr, "Erreur", "Impossible d'ouvrir le fichier");
        return;
    }

    auto start = std::chrono::high_resolution_clock::now();
    std::string line;
    std::getline(file, line);
    this->separator = line.find(';') != std::string::npos ? ';' : ',';
    this->headers = normalizeColumnNames(QString::fromStdString(line).split(this->separator));
    addLog("Colonnes : " + this->headers.join(", ") + " (séparateur : " + this->separator + ")" +
           " (nombre de colonnes : " + QString::number(this->headers.size()) + ")");
    addLog("Insertion des données...");

    this->ui->progressBar->setVisible(true);
    this->ui->progressBar->setValue(0);
    this->database.transaction();

    if (!this->ui->methodeBox->isChecked()) this->dropAndCreateTable();

    QSqlQuery query;
    QString insertSQL = "INSERT INTO " + this->ui->tableName->text() + " (";
    insertSQL += this->headers.join(", ") + ") VALUES (";
    insertSQL += QString("?, ").repeated(this->headers.size());
    insertSQL.chop(2);
    insertSQL += ")";

    if (!query.prepare(insertSQL)) {
        addLog("Erreur lors de la préparation de l'insertion : " + query.lastError().text());
        QMessageBox::critical(nullptr, "Erreur", "Impossible de préparer l'insertion : " + query.lastError().text());
        return;
    }

    const int batchSize = 10000;
    unsigned int rowCount = 0;

    while (std::getline(file, line)) {
        QStringList values = QString::fromStdString(line).split(this->separator);
        for (int i = 0; i < values.size(); ++i) {
            query.bindValue(i, values[i]);
        }
        if (!query.exec()) {
            addLog("Erreur lors de l'insertion de la ligne : " + query.lastError().text());
        }

        if (++rowCount % batchSize == 0) {
            database.commit();
            database.transaction();
            this->ui->progressBar->setValue(static_cast<int>((rowCount / static_cast<double>(batchSize)) * 100));
        }
    }

    database.commit();

    // Optimisation de la table pour réduire l'espace disque
    this->alterTable();

    addLog("Temps d'exécution : " +
           QString::number(std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::high_resolution_clock::now() - start).count()) + " ms");

    this->ui->progressBar->setValue(100);
    this->ui->progressBar->setVisible(false);
}

void InsertWindow::alterTable() {
    QSqlQuery query;
    QStringList alterStatements;
    for (const auto &header: this->headers) {
        QString sql = QString("SELECT MAX(LENGTH(%1)) FROM %2").arg(header).arg(this->ui->tableName->text());
        if (!query.exec(sql)) {
            addLog("Erreur lors de la récupération de la longueur maximale de la colonne " + header + " : " +
                   query.lastError().text());
            return;
        }
        if (query.next()) {
            int maxLength = query.value(0).toInt();
            if (maxLength == 0) {
                maxLength = 1;
            }
            QString alterSQL;
            switch (this->database.driverName().toStdString()[1]) {
                case 'S':
                    alterSQL = QString("%1 VARCHAR(%2)").arg(header).arg(maxLength);
                    break;
                case 'M':
                    alterSQL = QString("ALTER TABLE %1 MODIFY COLUMN %2 VARCHAR(%3)").arg(
                            this->ui->tableName->text()).arg(header).arg(maxLength);
                    break;
                case 'P':
                    alterSQL = QString("ALTER TABLE %1 ALTER COLUMN %2 TYPE VARCHAR(%3)").arg(
                            this->ui->tableName->text()).arg(header).arg(maxLength);
                    break;
                default:
                    addLog("Le driver de la base de données n'est pas supporté");
                    break;
            }
            alterStatements.append(alterSQL);
        }
    }

    if (database.driverName() == "QSQLITE") {
        QString createTableSQL = "CREATE TABLE " + this->ui->tableName->text() + "_new (";
        for (const auto &alterSQL: alterStatements) {
            createTableSQL += alterSQL + ", ";
        }
        createTableSQL.chop(2);
        createTableSQL += ")";

        QString copyDataSQL = QString("INSERT INTO %1_new SELECT * FROM %1").arg(this->ui->tableName->text());
        QString dropOldTableSQL = QString("DROP TABLE %1").arg(this->ui->tableName->text());
        QString renameTableSQL = QString("ALTER TABLE %1_new RENAME TO %1").arg(this->ui->tableName->text());

        if (!query.exec(createTableSQL)) {
            addLog("Erreur lors de la création de la nouvelle table : " + query.lastError().text());
            return;
        }
        if (!query.exec(copyDataSQL)) {
            addLog("Erreur lors de la copie des données : " + query.lastError().text());
            return;
        }
        if (!query.exec(dropOldTableSQL)) {
            addLog("Erreur lors de la suppression de l'ancienne table : " + query.lastError().text());
            return;
        }
        if (!query.exec(renameTableSQL)) {
            addLog("Erreur lors du renommage de la nouvelle table : " + query.lastError().text());
            return;
        }
    } else {
        for (const auto &alterSQL: alterStatements) {
            if (!query.exec(alterSQL)) {
                addLog("Erreur lors de la modification de la colonne : " + query.lastError().text());
                return;
            }
        }
    }
}

void InsertWindow::dropAndCreateTable() {
    QSqlQuery query;
    QString sql;

    sql = "DROP TABLE IF EXISTS " + this->ui->tableName->text() + ";";
    if (!query.exec(sql)) {
        addLog("Erreur lors de la suppression de la table : " + query.lastError().text());
        return;
    }

    sql = "CREATE TABLE " + this->ui->tableName->text() + " (";
    for (int i = 0; i < this->headers.size(); ++i) {
        sql += this->headers[i] + " TEXT";
        if (i < this->headers.size() - 1) sql += ", ";
    }
    sql += ")";

    if (!query.exec(sql)) {
        addLog("Erreur lors de la création de la table : " + query.lastError().text());
        QMessageBox::critical(nullptr, "Erreur", "Impossible de créer la table : " + query.lastError().text());
        return;
    }
}

void InsertWindow::addToolbar() {

    QToolBar *toolBar = addToolBar("Fichier");
    this->openButton = new QPushButton("Ouvrir", this);
    this->insertSqlButton = new QPushButton("Insérer", this);
    this->clearButton = new QPushButton("Effacer", this);

    toolBar->addWidget(openButton);
    toolBar->addWidget(insertSqlButton);
    toolBar->addWidget(clearButton);
    toolBar->setFloatable(true);
    toolBar->setMovable(true);
    toolBar->setAllowedAreas(
            Qt::TopToolBarArea | Qt::BottomToolBarArea | Qt::LeftToolBarArea | Qt::RightToolBarArea);
    toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    addToolBar(toolBar);

    // Ajouter du style CSS à la toolbar pour espacer les boutons
    toolBar->setStyleSheet(
            "QToolBar {background-color: #2196F3; color: white; padding: 5px;}"
            "QToolBar::this->separator {background-color: white; width: 5px;}");

    openButton->setIcon(QIcon::fromTheme("document-open"));
    openButton->setToolTip("Ouvrir un fichier");
    insertSqlButton->setToolTip("Insérer le contenu du fichier dans la base de données");
    insertSqlButton->setIcon(QIcon::fromTheme("insert-table"));
    clearButton->setToolTip("Effacer le tableau");
    clearButton->setIcon(QIcon::fromTheme("edit-clear"));
    clearButton->setText("Effacer");
    clearButton->setEnabled(false);
    clearButton->setVisible(false);

    // Ajouter un style matériel à la fenêtre
    setStyleSheet("QMenuBar {background-color: #2196F3; color: white;}"
                  "QMenuBar::item:selected {background-color: #1976D2;}"
                  "QHeaderView::section {background-color: #2196F3; color: white;}"
                  "QTableWidget::item:selected {background-color: #BBDEFB; color: black;}"
                  "QTableWidget::item:selected:!active {color: black;}"
                  "QTableWidget {text-align: center;}"
                  "QToolBar {background-color: #2196F3; color: white;}"
                  "QToolBar::this->separator {background-color: white; width: 5px;}"
                  "QPushButton {background-color: #2196F3; color: white; border: none; padding: 5px 10px;}"
                  "QPushButton:hover {background-color: #1976D2;}"
                  "QPushButton:pressed {background-color: #0D47A1;}");

    clearButton->setStyleSheet(
            "QPushButton {background-color: #f44336; color: white; border: none; padding: 5px 10px; border-radius: 5px;font-weight: bold;}"
            "QPushButton:hover {background-color: #d32f2f;}"
            "QPushButton:pressed {background-color: #b71c1c;}");

    openButton->setStyleSheet(
            "QPushButton {background-color: #2196F3; color: white; border: none; padding: 5px 10px; border-radius: 5px; font-weight: bold;}"
            "QPushButton:hover {background-color: #1976D2;}"
            "QPushButton:pressed {background-color: #0D47A1;}");

    insertSqlButton->setStyleSheet(
            "QPushButton {background-color: #4CAF50; color: white; border: none; padding: 5px 10px; border-radius: 5px; font-weight: bold;}"
            "QPushButton:hover {background-color: #388E3C;}"
            "QPushButton:pressed {background-color: #2E7D32;}");
}

void InsertWindow::clear_table() {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Effacer le fichier",
                                  "Êtes-vous sûr de vouloir effacer le fichier ?",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::No) {
        return;
    }

    this->fileName.clear();
    this->ui->fileName->clear();
    this->ui->tableName->clear();
    this->ui->portInput->clear();
    this->ui->databaseInput->clear();
    this->ui->passwordInput->clear();
    this->ui->userInput->clear();
    this->ui->hostInput->clear();
}

void InsertWindow::addLog(const QString &message) {
    this->ui->debugEdit->append(message);
}

void InsertWindow::set_connected(const bool connected) {
    if (connected) {
        this->ui->stateButton->setIcon(QIcon::fromTheme("network-connect"));
        this->ui->stateButton->setToolTip("Connecté");
        this->ui->stateButton->setStyleSheet(
                "QPushButton {background-color: #4CAF50; color: white; border: none; padding: 5px 10px; border-radius: 5px; font-weight: bold;}"
                "QPushButton:hover {background-color: #388E3C;}"
                "QPushButton:pressed {background-color: #2E7D32;}");
        return;
    }
    this->ui->stateButton->setIcon(QIcon::fromTheme("network-disconnect"));
    this->ui->stateButton->setToolTip("Non connecté");
    this->ui->stateButton->setStyleSheet(
            "QPushButton {background-color: #FFC107; color: white; border: none; padding: 5px 10px; border-radius: 5px; font-weight: bold;}"
            "QPushButton:hover {background-color: #FFA000;}"
            "QPushButton:pressed {background-color: #FF8F00;}");
}

bool InsertWindow::connect_to_db() {

    if (database.isOpen()) {
        QMessageBox::warning(this, "Déjà connecté", "Vous êtes déjà connecté à une base de données");
        return false;
    }

    QString host = this->ui->hostInput->text();
    QString user = this->ui->userInput->text();
    QString password = this->ui->passwordInput->text();
    QString database_name = this->ui->databaseInput->text();
    QString port = this->ui->portInput->text();

//    if (host.isEmpty() || user.isEmpty() || password.isEmpty() || database.isEmpty() || port.isEmpty()) {
//        QMessageBox::warning(this, "Champs vides",
//                             "Veuillez remplir tous les champs pour vous connecter à la base de données");
//        return false;
//    }

    addLog("Connexion à la base de données avec les paramètres suivants :");
    addLog("Hôte: " + host);
    addLog("Utilisateur: " + user);
    addLog("Base de données: " + database_name);
    addLog("Port: " + port);

    this->database = QSqlDatabase::addDatabase("QSQLITE");
    this->database.setDatabaseName("../database.sqlite");

    if (!this->database.open()) {
        addLog("Impossible de se connecter à la base de données");
        return false;
    }

    set_connected(this->database.isOpen());
    return true;
}
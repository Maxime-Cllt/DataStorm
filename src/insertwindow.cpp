#include "insertwindow.h"
#include "ui_insertwindow.h"
#include "datastorm.h"
#include <QToolBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <iostream>
#include <fstream>
#include <QInputDialog>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTimer>

InsertWindow::InsertWindow(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::InsertWindow) {
    this->ui->setupUi(this);

    this->setWindowTitle("DataStorm");
    this->setWindowIcon(QIcon::fromTheme("insert-table"));

    this->addMenuBar();
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

    this->ui->database_box->setStyleSheet(
            "QGroupBox {background-color: #f5f5f5; border: 1px solid #f5f5f5; border-radius: 5px; padding: 5px;}");

    this->ui->database_path_opener->setStyleSheet(
            "QLineEdit {background-color: #f5f5f5; border: 1px solid #f5f5f5; border-radius: 5px; padding: 5px;}");
    this->ui->database_path_opener->setToolTip("Chemin du fichier SQLite");
    this->ui->database_path_opener->setIcon(QIcon::fromTheme("folder"));

    QStringList drivers = QSqlDatabase::drivers();
    this->ui->database_box->addItem("");
    for (const QString &driver: drivers) {
        this->ui->database_box->addItem(driver);
    }

    // SLOT connections
    connect(insertSqlButton, &QPushButton::clicked, this, &InsertWindow::insertFile);
    connect(clearButton, &QPushButton::clicked, this, &InsertWindow::clearTable);
    connect(openButton, &QPushButton::clicked, this, &InsertWindow::openFile);
    connect(this->ui->connectButton, &QPushButton::clicked, this, &InsertWindow::connectToDb);
    connect(this->ui->connectButton, &QPushButton::pressed, [this]() {
        QTimer::singleShot(5000, [this]() {
            if (!this->database.isOpen()) return;

            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, "Déconnexion",
                                          "Voulez-vous vous déconnecter de la base de données ?",
                                          QMessageBox::Yes | QMessageBox::No);
            if (reply == QMessageBox::Yes) {
                this->database.close();
                this->setConnected(false);
                this->addLog("Déconnecté de la base de données");
            }

        });
    });
    connect(this->ui->database_box, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](const int index) {

        this->connectionType = this->ui->database_box->currentText();

        if (connectionType == "QSQLITE") {
            this->ui->portInput->setText("");
            this->ui->sqlite_path_input->setVisible(true);
            this->ui->sqlite_path_input->setEnabled(true);
            this->ui->sqlite_input_label->setVisible(true);
            this->ui->database_path_opener->setVisible(true);
            return;
        } else if (connectionType == "QMARIADB") {
            this->ui->portInput->setText("3307");
        } else if (connectionType == "QMYSQL") {
            this->ui->portInput->setText("3306");
        } else if (connectionType == "QODBC") {
            this->ui->portInput->setText("1433");
        } else if (connectionType == "QPSQL") {
            this->ui->portInput->setText("5432");
        }

        this->ui->sqlite_path_input->setVisible(false);
        this->ui->sqlite_path_input->setEnabled(false);
        this->ui->sqlite_input_label->setVisible(false);
        this->ui->database_path_opener->setVisible(false);
    });
    connect(this->ui->database_path_opener, &QPushButton::clicked, [this]() {
        QString path = QFileDialog::getOpenFileName(this, "Ouvrir un fichier SQLite", qApp->applicationDirPath(),
                                                    "SQLite Files (*.sqlite *.db)");
        if (path.isEmpty()) return;
        this->ui->sqlite_path_input->setText(path);
    });
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

void InsertWindow::openFile() {

    this->fileName = QFileDialog::getOpenFileName(this, "Ouvrir un fichier", qApp->applicationDirPath(),
                                                  "CSV Files (*.csv);;Excel Files (*.xlsx)");
    if (this->fileName.isEmpty()) return;

    const QStringList parts = this->fileName.split("/");
    QFile file(this->fileName);
    this->ui->fileName->setAlignment(Qt::AlignCenter);
    this->ui->fileName->setText(
            "Fichier: " + parts[parts.size() - 1] + " (" + QString::number(file.size() / 1024) + " Ko)");

    this->ui->tableName->setText(get_name_for_table(this->fileName));
    addLog("Fichier ouvert : " + this->fileName);
    file.close();
}

void InsertWindow::insertFile() {
    if (this->fileName.isEmpty()) {
        QMessageBox::warning(nullptr, "Fichier non ouvert", "Veuillez ouvrir un fichier avant de l'insérer");
        return;
    }
    if (!database.isOpen()) {
        QMessageBox::warning(nullptr, "Base de données non connectée",
                             "Veuillez vous connecter à une base de données avant d'insérer le fichier");
        return;
    }

    this->ui->progressBar->setVisible(true);
    this->ui->progressBar->setValue(0);

    const std::string type = getFileExtension(this->fileName.toStdString());
    if (type == "csv") {
        this->loadCSV();
    } else QMessageBox::warning(nullptr, "Fichier non supporté", "Le fichier n'est pas supporté");
    this->ui->progressBar->setValue(100);
}

void InsertWindow::loadCSV() {
    std::ifstream file(this->fileName.toStdString());
    if (!file.is_open()) {
        addLog("Impossible d'ouvrir le fichier");
        QMessageBox::critical(nullptr, "Erreur", "Impossible d'ouvrir le fichier");
        return;
    }

    const auto start = std::chrono::high_resolution_clock::now();

    std::string line;
    std::getline(file, line);
    this->separator = line.find(';') != std::string::npos ? ';' : ',';
    this->headers = normalizeColumnNames(QString::fromStdString(line).split(this->separator));
    addLog("Colonnes : " + this->headers.join(", ") + " (séparateur : " + this->separator + ")" +
           " (nombre de colonnes : " + QString::number(this->headers.size()) + ")");
    addLog("Insertion des données...");

    this->database.transaction();

    if (!this->ui->methodeBox->isChecked()) this->dropAndCreateTable();

    addLog("Temps création : " +
           QString::number(std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::high_resolution_clock::now() - start).count()) + " ms");

    QSqlQuery query(this->database);
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

    const short batchSize = 1000;
    unsigned int rowCount = 0;
    QMap<QString, int> maxLengths;
    for (const auto &header: this->headers) {
        maxLengths[header] = 0;
    }
    QStringList values;

    while (std::getline(file, line)) {
        values = QString::fromStdString(line).split(this->separator, Qt::KeepEmptyParts);

        for (int i = 0; i < values.size(); ++i) {
            const QString &value = values[i];
            const int length = static_cast<int>(value.length());
            if (length > maxLengths[this->headers[i]]) {
                maxLengths[this->headers[i]] = length;
            }
            query.bindValue(i, value);
        }

        if (!query.exec()) {
            this->addLog("Erreur lors de l'insertion de la ligne : " + query.lastError().text());
        }

        if (++rowCount == batchSize) {
            database.commit();
            database.transaction();
//            QMetaObject::invokeMethod(this->ui->progressBar, "setValue", Qt::QueuedConnection,
//                                      Q_ARG(int, (int) ((rowCount / (double) this->headers.size()) * 100)));
        }
    }

    database.commit();

    addLog("Temps insertion : " +
           QString::number(std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::high_resolution_clock::now() - start).count()) + " ms");

    // Optimisation de la table pour réduire l'espace disque en modifiant la taille des colonnes en arrière-plan
    this->alterTable(maxLengths);

    addLog("Temps d'exécution total: " +
           QString::number(std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::high_resolution_clock::now() - start).count()) + " ms");

    file.close();
    query.clear();
    query.finish();
}

void InsertWindow::alterTable(const QMap<QString, int> &maxLenghtColumns) {
    const QString &tableName = this->ui->tableName->text().trimmed();
    QStringList columnDefinitions;
    QSqlQuery query(this->database);

    for (const auto &header: this->headers) {
        const int maxLength = maxLenghtColumns[header];
        QString columnDef = header + " VARCHAR(" + QString::number(maxLength) + ")";
        columnDefinitions.append(columnDef);
    }

    QString createTableSQL = "CREATE TABLE " + tableName + "_new (";
    for (const auto &columnDef: columnDefinitions) {
        createTableSQL += columnDef + ", ";
    }
    createTableSQL.chop(2);
    createTableSQL += ")";

    const QString copyDataSQL = QString("INSERT INTO %1_new SELECT * FROM %1").arg(tableName);
    const QString dropOldTableSQL = QString("DROP TABLE %1").arg(tableName);
    const QString renameTableSQL = QString("ALTER TABLE %1_new RENAME TO %1").arg(tableName);

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
    query.clear();
    query.finish();
}

void InsertWindow::dropAndCreateTable() {
    QSqlQuery query(this->database);
    QString sql;
    const QString tableName = this->ui->tableName->text().trimmed();

    sql = "DROP TABLE IF EXISTS " + tableName + ";";
    if (!query.exec(sql)) {
        addLog("Erreur lors de la suppression de la table : " + query.lastError().text());
        return;
    }

    const QMap<QString, QString> sqlTemporaryTables = {
            {"QSQLITE",  "CREATE TEMP TABLE "},
            {"QMARIADB", "CREATE TEMPORARY TABLE "},
            {"QMYSQL",   "CREATE TEMPORARY TABLE "},
            {"QODBC",    "CREATE TEMPORARY TABLE "},
            {"QPSQL",    "CREATE TEMPORARY TABLE "}
    };

    sql = sqlTemporaryTables[this->connectionType] + tableName + " (";
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
    query.clear();
    query.finish();
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

void InsertWindow::addMenuBar() {
    std::unique_ptr<QMenuBar> menuBar = std::make_unique<QMenuBar>(this);
    QMenu *fileMenu = menuBar->addMenu("Réglages");
    QAction *saveConfig = fileMenu->addAction("Sauvegarder la configuration");
    QAction *loadConfig = fileMenu->addAction("Charger la configuration");
    connect(saveConfig, &QAction::triggered, this, &InsertWindow::saveConfig);
    connect(loadConfig, &QAction::triggered, this, &InsertWindow::loadConfig);

    menuBar->setStyleSheet(
            "QMenuBar {background-color: #2196F3; color: white;}"
            "QMenuBar::item:selected {background-color: #1976D2;}"
            "QMenu {background-color: #2196F3; color: white;}"
            "QMenu::item:selected {background-color: #1976D2;}"
            "QMenu::separator {background-color: white; width: 5px;}"
            "QMenuBar::item {padding: 5px 10px;}"
            "QMenuBar::item:selected {background-color: #1976D2;}"
            "QMenuBar::item:pressed {background-color: #0D47A1;}"
            "QMenuBar::item {background-color: #2196F3; color: white;}"
            "QMenuBar::item:selected {background-color: #1976D2;}"
            "QMenuBar::item:pressed {background-color: #0D47A1;}"
            "QMenuBar::item {background-color: #2196F3; color: white;}"
            "QMenuBar::item:selected {background-color: #1976D2;}"
            "QMenuBar::item:pressed {background-color: #0D47A1;}"
            "QMenuBar::item {background-color: #2196F3; color: white;}"
            "QMenuBar::item:selected {background-color: #1976D2;}"
            "QMenuBar::item:pressed {background-color: #0D47A1;}"
            "QMenuBar::item {background-color: #2196F3; color: white;}"
            "QMenuBar::item:selected {background-color: #1976D2;}"
            "QMenuBar::item:pressed {background-color: #0D47A1;}"
            "QMenuBar::item {background-color: #2196F3; color: white;}"
            "QMenuBar::item:selected {background-color: #1976D2;}"
            "QMenuBar::item:pressed {background-color: #0D47A1;}"
            "QMenuBar::item {background-color: #2196F3; color: white;}"
            "QMenuBar::item:selected {background-color: #1976D2;}"
            "QMenuBar::item:pressed {background-color: #0D47A1;}"
            "QMenuBar::item {background-color: #2196F3; color: white;}"
            "QMenuBar::item:selected {background-color: #1976D2;}"
            "QMenuBar::item:pressed {background-color: #0D47A1;}"
            "QMenuBar::item {background-color: #2196F3; color: white;}"
    );
    setMenuBar(menuBar.release());
}

void InsertWindow::saveConfig() {
    bool ok;
    const QString name = QInputDialog::getText(this, "Nom de la configuration",
                                               "Nom de la configuration:", QLineEdit::Normal,
                                               "", &ok);
    if (!ok || name.isEmpty()) {
        return;
    }

    QFile file(qApp->applicationDirPath() + "/config/databse_config.json");

    if (!file.exists()) {
        QDir dir(qApp->applicationDirPath() + "/config");
        if (!dir.exists()) {
            dir.mkpath(".");
        }
        file.open(QIODevice::WriteOnly);
        file.close();
    }

    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, "Erreur", "Impossible d'ouvrir le fichier de configuration");
        return;
    }

    QJsonObject json;
    json["name"] = name;
    json["driver"] = this->ui->database_box->currentText();
    json["host"] = this->ui->hostInput->text();
    json["user"] = this->ui->userInput->text();
    json["password"] = this->ui->passwordInput->text();
    json["database"] = this->ui->databaseInput->text();
    json["port"] = this->ui->portInput->text();
    json["connectionType"] = this->ui->database_box->currentText();
    json["fileName"] = this->fileName;
    json["tableName"] = this->ui->tableName->text();
    json["sqlitePath"] = this->ui->sqlite_path_input->text();

    QJsonDocument doc(json);
    file.write(doc.toJson());
    file.close();

    QMessageBox::information(this, "Configuration sauvegardée", "La configuration a été sauvegardée avec succès");
}

void InsertWindow::loadConfig() {
    QFile file(qApp->applicationDirPath() + "/config/databse_config.json");
    if (!file.exists()) {
        QMessageBox::warning(this, "Fichier de configuration introuvable",
                             "Aucune configuration n'a été sauvegardée");
        return;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Erreur", "Impossible d'ouvrir le fichier de configuration");
        return;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    const QJsonObject json = doc.object();

    this->ui->database_box->setCurrentText(json["driver"].toString());
    this->ui->hostInput->setText(json["host"].toString());
    this->ui->userInput->setText(json["user"].toString());
    this->ui->passwordInput->setText(json["password"].toString());
    this->ui->databaseInput->setText(json["database"].toString());
    this->ui->portInput->setText(json["port"].toString());
    this->ui->database_box->setCurrentText(json["connectionType"].toString());
    this->ui->sqlite_path_input->setText(json["sqlitePath"].toString());

    this->connectToDb();
}

void InsertWindow::clearTable() {
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

void InsertWindow::setConnected(const bool connected) {
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

bool InsertWindow::connectToDb() {
    try {
        if (this->database.isOpen()) {
            QMessageBox::warning(this, "Déjà connecté", "Vous êtes déjà connecté à une base de données");
            return false;
        }

        if (this->connectionType == "QSQLITE") {
            QString path = this->ui->sqlite_path_input->text();
            if (path.isEmpty()) {
                QMessageBox::warning(this, "Champ vide",
                                     "Veuillez entrer le chemin du fichier SQLite pour vous connecter à la base de données");
                return false;
            }

            this->database = QSqlDatabase::addDatabase(this->connectionType);
            this->database.setDatabaseName(path.trimmed());
            this->addLog("Connexion à la base de données SQLite avec le chemin : " + path);

        } else {
            const QString host = this->ui->hostInput->text();
            const QString user = this->ui->userInput->text();
            const QString password = this->ui->passwordInput->text();
            const QString database_name = this->ui->databaseInput->text();
            const QString port = this->ui->portInput->text();

            if (host.isEmpty() || user.isEmpty() || password.isEmpty() || database_name.isEmpty() || port.isEmpty()) {
                QMessageBox::warning(this, "Champs vides",
                                     "Veuillez remplir tous les champs pour vous connecter à la base de données");
                return false;
            }

            this->database = QSqlDatabase::addDatabase(this->connectionType);
            this->database.setHostName(host.trimmed());
            this->database.setUserName(user.trimmed());
            this->database.setPassword(password.trimmed());
            this->database.setDatabaseName(database_name.trimmed());
            this->database.setPort((port.trimmed()).toInt());

            addLog("Tentative de connexion à la base de données...");
        }

        if (!this->database.open()) {
            addLog("Impossible de se connecter à la base de données");
            return false;
        }

        this->setConnected(this->database.isOpen());
        return this->database.isOpen();
    }
    catch (const std::exception &e) {
        this->addLog("Erreur lors de la connexion à la base de données : " + QString(e.what()));
        QMessageBox::critical(this, "Erreur", e.what());
        return false;
    }
}
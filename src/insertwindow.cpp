#include "insertwindow.h"
#include "ui_insertwindow.h"
#include "Util.h"
#include <QToolBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <iostream>
#include <iterator>
#include <QTextStream>
#include <QThread>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <vector>

InsertWindow::InsertWindow(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::InsertWindow) {
    this->ui->setupUi(this);

    addToolbar();

    // Bouton pour arrêter le traitement
    this->ui->abortButton->setVisible(false);
    this->ui->abortButton->setEnabled(false);
    this->ui->abortButton->setIcon(QIcon::fromTheme("process-stop"));
    this->ui->abortButton->setToolTip("Arrêter le traitement en cours");
    this->ui->abortButton->setStyleSheet(
            "QPushButton {background-color: #f44336; color: white; border: none; padding: 5px 10px; border-radius: 5px;font-weight: bold;}"
            "QPushButton:hover {background-color: #d32f2f;}"
            "QPushButton:pressed {background-color: #b71c1c;}");
    this->ui->abortButton->setEnabled(false);

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
            "QProgressBar::chunk {background-color: #4CAF50; width: 20px;}");

    this->ui->methodeBox->setText("APPEND TO TABLE");
    this->ui->methodeBox->setStyleSheet(
            "QLineEdit {background-color: #f5f5f5; border: 1px solid #f5f5f5; border-radius: 5px; padding: 5px;}");
    this->ui->methodeBox->setEnabled(false);
    this->ui->methodeBox->setVisible(false);


    // SLOT connections
    connect(insertSqlButton, &QPushButton::clicked, this, &InsertWindow::insert_file);
    connect(clearButton, &QPushButton::clicked, this, &InsertWindow::clear_table);
    connect(openButton, &QPushButton::clicked, this, [this] {
        open_file();
    });
    connect(this->ui->connectButton, &QPushButton::clicked, this, &InsertWindow::connect_to_db);
}

InsertWindow::~InsertWindow() {
    delete this->ui;
    delete clearButton;
    delete insertSqlButton;
    delete openButton;
}

QString get_name_for_table(const QString &fileName) {
    QStringList parts = fileName.contains("/") ? fileName.split("/") : fileName.split("\\");
    QString name = parts[parts.size() - 1];
    name = name.split(".")[0];
    name = name.toLower();
    name = name.replace(" ", "_");
    return name;
}

void InsertWindow::open_file() {
    fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "",
                                            tr("CSV Files (*.csv);;All Files (*)"));
    if (fileName.isEmpty()) return;

    QStringList parts = fileName.split("/");
    QFile file(fileName);
    this->ui->fileName->setAlignment(Qt::AlignCenter);
    this->ui->fileName->setText(
            "Fichier: " + parts[parts.size() - 1] + " (" + QString::number(file.size() / 1024) + " Ko)");

    this->ui->tableName->setText(get_name_for_table(fileName));
    this->ui->methodeBox->setEnabled(false);
    this->ui->methodeBox->setVisible(false);
    addLog("Fichier ouvert : " + fileName);
}

void InsertWindow::insert_file() {
    if (fileName.isEmpty()) {
        QMessageBox::warning(this, "Fichier non ouvert", "Veuillez ouvrir un fichier avant de l'insérer");
        return;
    }

    if (!database.isOpen()) {
        QMessageBox::warning(this, "Base de données non connectée",
                             "Veuillez vous connecter à une base de données avant d'insérer le fichier");
        return;
    }

    this->ui->abortButton->setEnabled(true);
    this->ui->abortButton->setVisible(true);

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Erreur", "Impossible d'ouvrir le fichier : " + file.errorString());
        return;
    }

    auto start = std::chrono::high_resolution_clock::now();
    QTextStream in(&file);

    QString line = in.readLine();
    char separator = line.contains(";") ? ';' : ',';
    QStringList columns = normalizeColumnNames(line.split(separator));
    addLog("Colonnes : " + columns.join(", ") + " (séparateur : " + separator + ")" + " (nombre de colonnes : " +
           QString::number(columns.size()) + ")");
    addLog("Insertion des données...");
    this->ui->progressBar->setVisible(true);
    ui->progressBar->setValue(0);

    this->database.transaction();
    QSqlQuery query;
    QString sql;

    if (!this->ui->methodeBox->isChecked()) {
        sql = "DROP TABLE IF EXISTS " + this->ui->tableName->text() + ";";
        if (!query.exec(sql)) {
            addLog("Erreur lors de la suppression de la table : " + query.lastError().text());
            QMessageBox::critical(this, "Erreur", "Impossible de supprimer la table : " + query.lastError().text());
            return;
        }
        this->database.commit();
    }

    // Créer la table
    sql = "CREATE TABLE " + this->ui->tableName->text() + " (";
    for (unsigned int i = 0; i < columns.size(); i++) {
        sql += columns[i] + " TEXT";
        if (i < columns.size() - 1) sql += ", ";
    }
    sql += ")";

    if (!query.exec(sql)) {
        addLog("Erreur lors de la création de la table : " + query.lastError().text());
        QMessageBox::critical(this, "Erreur", "Impossible de créer la table : " + query.lastError().text());
        return;
    }
    this->database.commit();
    this->ui->progressBar->setValue(5);

    const unsigned int lineCount = countLinesInFile(fileName);
    this->addLog("Nombre de lignes dans le fichier : " + QString::number(lineCount));

    // insere les données en background pour ne pas bloquer l'interface
    QFuture<void> future = QtConcurrent::run([=, this]() {
        try {
            QFile file(fileName);
            if (!file.open(QIODevice::ReadOnly)) {
                addLog("Erreur: Impossible d'ouvrir le fichier en background");
                return;
            }
            QTextStream in(&file);
            QSqlQuery query(this->database);
            QString sql;
            int i = 0;

            while (!in.atEnd()) {
                QString line = in.readLine();
                QStringList values = line.split(separator);

                if (values.size() != columns.size()) {
                    addLog("Erreur : la ligne " + QString::number(i) + " ne contient pas le bon nombre de colonnes");
                    continue;
                }

                if (!query.exec(insertData(ui->tableName->text(), columns, values))) {
                    addLog("Erreur lors de l'insertion de la ligne " + QString::number(i) + " : " +
                           query.lastError().text());
                    continue;
                }

                i++;
                int progress = static_cast<int>(static_cast<double>(i) / static_cast<double>(lineCount) * 100);
                QMetaObject::invokeMethod(this->ui->progressBar, "setValue", Q_ARG(int, progress));
            }
            this->database.commit();

            addLog("Données insérées avec succès");
            this->ui->progressBar->setValue(100);
            this->ui->abortButton->setEnabled(false);
            this->ui->abortButton->setVisible(false);
            auto end = std::chrono::high_resolution_clock::now();
            this->addLog(
                    "Temps d'exécution : " + QString::number(
                            (double) std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() /
                            1000.0) +
                    " s");
        } catch (const std::exception &e) {
            std::cerr << e.what() << std::endl;
            addLog("Erreur lors de l'insertion des données : " + QString::fromStdString(e.what()));
        }
    });


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
            "QToolBar::separator {background-color: white; width: 5px;}");

    openButton->setIcon(QIcon::fromTheme("document-open"));
    openButton->setToolTip("Ouvrir un fichier");
    insertSqlButton->setToolTip("Insérer le contenu du fichier dans la base de données");
    insertSqlButton->setIcon(QIcon::fromTheme("insert-object"));
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
                  "QToolBar::separator {background-color: white; width: 5px;}"
                  "QPushButton {background-color: #2196F3; color: white; border: none; padding: 5px 10px;}"
                  "QPushButton:hover {background-color: #1976D2;}"
                  "QPushButton:pressed {background-color: #0D47A1;}");

    clearButton->setStyleSheet(
            "QPushButton {background-color: #f44336; color: white; border: none; padding: 5px 10px; border-radius: 5px;font-weight: bold;}"
            "QPushButton:hover {background-color: #d32f2f;}"
            "QPushButton:pressed {background-color: #b71c1c;}");

    openButton->setStyleSheet(
            "QPushButton {background-color: #4CAF50; color: white; border: none; padding: 5px 10px; border-radius: 5px; font-weight: bold;}"
            "QPushButton:hover {background-color: #388E3C;}"
            "QPushButton:pressed {background-color: #2E7D32;}");

    insertSqlButton->setStyleSheet(
            "QPushButton {background-color: #FFC107; color: white; border: none; padding: 5px 10px; border-radius: 5px; font-weight: bold;}"
            "QPushButton:hover {background-color: #00CED1;}"
            "QPushButton:pressed {background-color: #00BFFF;}");
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
#include "DatabaseConfig.h"
#include "ui_DatabaseConfig.h"

DatabaseConfig::DatabaseConfig(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DatabaseConfig)
{
    ui->setupUi(this);
}

DatabaseConfig::~DatabaseConfig()
{
    delete ui;
}

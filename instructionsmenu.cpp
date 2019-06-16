#include "instructionsmenu.h"
#include "ui_instructionsmenu.h"

instructionsMenu::instructionsMenu(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::instructionsMenu)
{
    ui->setupUi(this);
}

instructionsMenu::~instructionsMenu()
{
    delete ui;
}

void instructionsMenu::on_buttonBox_accepted()
{
    this->close();
}

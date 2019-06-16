#ifndef INSTRUCTIONSMENU_H
#define INSTRUCTIONSMENU_H

#include <QDialog>

namespace Ui {
class instructionsMenu;
}

class instructionsMenu : public QDialog
{
    Q_OBJECT

public:
    explicit instructionsMenu(QWidget *parent = 0);
    ~instructionsMenu();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::instructionsMenu *ui;
};

#endif // INSTRUCTIONSMENU_H

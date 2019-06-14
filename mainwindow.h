#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    bool loadInStrings(char *fileLocation);

    void on_listWidget_itemClicked(QListWidgetItem *item);

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_actionopen_triggered();

    void on_textEdit_textChanged();

    void on_actionexit_triggered();

    void on_actionrevert_changes_to_exe_triggered();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H

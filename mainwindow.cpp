#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "Utilites.h"
#include "EmbeddingCode.h"
#include "pch.h"
#include <sys/stat.h>
#include "QFileDialog"
#include <QMessageBox>
#include <QColor>
#include "Shlobj.h"
#include <QProcess>

bool loadBinary(const char* filePath,char*& fileData,int& fileLenght) {
    free(fileData);

    FILE* fileHandel;
    fopen_s(&fileHandel, filePath, "rb+");
    fseek(fileHandel, 0, SEEK_END);
    int tempFileLenght = ftell(fileHandel);
    fseek(fileHandel, 0, SEEK_SET);
    char* buffer = (char*)malloc(tempFileLenght);
    fread(buffer, 1, tempFileLenght, fileHandel);
    fclose(fileHandel);

    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)buffer;
    PIMAGE_FILE_HEADER FH = (PIMAGE_FILE_HEADER)(buffer + pDosHeader->e_lfanew + sizeof(DWORD));

    if(FH->Machine != (WORD)0x14c){// not 32 bit
        QMessageBox Msgbox;
        Msgbox.setText("Error - binary selected is not 32 bit");
        Msgbox.exec();
        return false;
    }

    fileData = buffer;
    fileLenght = tempFileLenght;
    return true;
}

QListWidgetItem * currentItem;
char* exeFileAndPath;
std::vector<stringReferenceClass> stringRefrences;
char* fileData;
int fileLenght;

bool MainWindow::loadInStrings(char* fileLocation){
    if(fileLocation==NULL)
        return false;

    std::string saveFilePath = fileLocation;
    saveFilePath.append(".txt");

    struct stat buffer;
    if (stat(saveFilePath.c_str(), &buffer) == 0)
        stringRefrences = loadData(saveFilePath);
    else {
        stringRefrences = getStringData(fileData);
        saveData(saveFilePath, stringRefrences);
    }

    for (auto& a : stringRefrences) {
        QListWidgetItem* item = new QListWidgetItem(a.string.c_str());

        if(a.stringReferenceLocations.size()>0)
            item->setForeground(Qt::darkGreen);//item->setBackground(QColor("#e7eecc"));
        else item->setForeground(Qt::darkRed);//item->setBackground(QColor("#7fc97f"));

        ui->listWidget->addItem(item);

        //QWidget *widget = new QWidget();
        //ui->listWidget->setItemWidget(item,widget);
    }

    return true;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_listWidget_itemClicked(QListWidgetItem * item)
{
    currentItem = item;
    ui->textEdit->setText(item->text());
}

void MainWindow::on_pushButton_clicked()
{
    if(exeFileAndPath == nullptr)
        return;

    QString path = QString(exeFileAndPath);

    QStringList args;

    args << "/select," << QDir::toNativeSeparators(path);

    QProcess *process = new QProcess(this);
    process->start("explorer.exe", args);

}

void MainWindow::on_pushButton_2_clicked()
{
    auto editString = [&](int index,std::string newString) {
        stringRefrences[index].changedString = newString;
        openSectionHeader(fileData, fileLenght, 0x99887766, ".rdata", IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ, stringRefrences);
        stringRefrences[index].editString(fileData);
    };

    for(int i =0;i<stringRefrences.size();i++){
        if(ui->listWidget->item(i)->text().remove(QChar::Null) != QString::fromStdString(stringRefrences[i].string).remove(QChar::Null))
            editString(i,ui->listWidget->item(i)->text().toStdString());
    };

    FILE* newFile;
    fopen_s(&newFile, exeFileAndPath,"wb");
    const char* words = exeFileAndPath;
    int error = errno;
    fwrite(fileData,1,fileLenght,newFile);
    fclose(newFile);
}

void MainWindow::on_actionopen_triggered()
{
    QString getFile = QFileDialog::getOpenFileName();
    if(getFile.isEmpty())
        return;

    std::string oldString;
    if(exeFileAndPath!=nullptr)
        oldString = exeFileAndPath;

    delete[] exeFileAndPath;
    QByteArray inUtf8 = getFile.toUtf8();
    exeFileAndPath = new char[inUtf8.size()+2]();
    memcpy(exeFileAndPath,(void*)inUtf8.data(),inUtf8.size());

    if(loadBinary(exeFileAndPath, fileData, fileLenght)==false){
        if(oldString.size()!=0)
            exeFileAndPath = (char*)oldString.c_str();
        return;
    }

    stringRefrences.clear();
    ui->listWidget->clear();
    currentItem = nullptr;
    ui->textEdit->setText("");

    loadInStrings(exeFileAndPath);
}

void MainWindow::on_textEdit_textChanged()
{
    if(currentItem != nullptr)
        currentItem->setText(ui->textEdit->toPlainText());
}

void MainWindow::on_actionexit_triggered()
{
    exit(0);
}

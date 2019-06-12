#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "Utilites.h"
#include "EmbeddingCode.h"
#include "pch.h"
#include <sys/stat.h>
#include "QFileDialog"

void loadBinary(const char* filePath,char*& fileData,int& fileLenght) {
    FILE* fileHandel;
    fopen_s(&fileHandel, filePath, "rb+");
    fseek(fileHandel, 0, SEEK_END);
    fileLenght = ftell(fileHandel);
    fseek(fileHandel, 0, SEEK_SET);
    fileData = (char*)malloc(fileLenght);
    fread(fileData, 1, fileLenght, fileHandel);
    fclose(fileHandel);
}

std::vector<stringReferenceClass> getStringData(char*& fileData) {
    std::vector<stringReferenceClass> stringRefrences;

    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)fileData;
    PIMAGE_FILE_HEADER FH = (PIMAGE_FILE_HEADER)(fileData + pDosHeader->e_lfanew + sizeof(DWORD));
    PIMAGE_OPTIONAL_HEADER OH = (PIMAGE_OPTIONAL_HEADER)(fileData + pDosHeader->e_lfanew + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER));
    PIMAGE_SECTION_HEADER SH = (PIMAGE_SECTION_HEADER)(fileData + pDosHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS));

    DWORD codeSectionAddress;
    DWORD sizeOfCodeSection;

    for (int i = 0; i < FH->NumberOfSections; i++) {
        char* sectionName = (char*)SH[i].Name;
        DWORD addresOfSectionData = SH[i].PointerToRawData;
        DWORD sizeOfSection = SH[i].Misc.VirtualSize;
        int currentIndexOfSections = i;

        //std::cout << sectionName << "\n" << "pointer to data " << std::hex << addresOfSectionData << "\n" << "size of section "<< sizeOfSection << "\n";
        std::cout << std::hex;

        if (strcmp(sectionName, ".text") == 0) {
            codeSectionAddress = addresOfSectionData;
            sizeOfCodeSection = sizeOfSection;
        }

        if (strcmp(sectionName, ".rdata") != 0)
            continue;

        int letterCounter = 0;
        std::string sString = "";
        int currentStringOffset;

        for (int i = addresOfSectionData; i < addresOfSectionData + sizeOfSection; i++) {
            if (isLetter(fileData[i])) {
                sString.insert(letterCounter, 1, (char)fileData[i]);

                letterCounter++;
            }
            else if (letterCounter != 0) {
                sString.insert(letterCounter, 1, (char)'\0');

                int symbolCount;
                float persentageSymbols;
                DWORD referenceToString = 0x0;
                DWORD addres;

                if (letterCounter < 2)
                    goto end;

                symbolCount = numberOfSymbols(sString.c_str());
                persentageSymbols = (float)((float)symbolCount / letterCounter);

                if (persentageSymbols >= 0.5)
                    goto end;

                // usless strings after the point RSDS
                if (sString.find("RSDS") == 0)
                    break;

                referenceToString = (i - letterCounter);
                addres = getVirtualAddressFromPyisical(referenceToString, addresOfSectionData, OH->ImageBase, SH[currentIndexOfSections].VirtualAddress);

                stringRefrences.emplace_back(referenceToString, addres, sString);

            end:
                sString.clear();
                letterCounter = 0;
            }
        }

    }

    auto spawnThreads = [&](int noOfThreads) {
        std::mutex mainMutex;
        int sizeForEachThread = sizeOfCodeSection / noOfThreads;
        std::vector<std::thread> threads(noOfThreads);
        for (int i = 0; i < noOfThreads; i++) {
            threads[i] = std::thread(findStringReference, std::ref(fileData), std::ref(stringRefrences), codeSectionAddress + (i*sizeForEachThread), sizeForEachThread, std::ref(mainMutex));
        }
        for (auto& th : threads) {
            th.join();
        }
    };
    spawnThreads(4);

    return stringRefrences;
}


QListWidgetItem * currentItem;
char* exeFileAndPath;
std::vector<stringReferenceClass> stringRefrences;
char* fileData;
int fileLenght;

void MainWindow::loadInStrings(char* fileLocation){
    if(fileLocation==NULL)
        return;

    std::string saveFilePath = fileLocation;
    saveFilePath.append(".txt");

    loadBinary(fileLocation, fileData, fileLenght);

    struct stat buffer;
    if (stat(saveFilePath.c_str(), &buffer) == 0)
        stringRefrences = loadData(saveFilePath);
    else {
        stringRefrences = getStringData(fileData);
        saveData(saveFilePath, stringRefrences);
    }

    for (auto& a : stringRefrences) {
        ui->listWidget->addItem(a.string.c_str());
    }

}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //exeFileAndPath = "C:\\Users\\Liam\\source\\repos\\cppStringEditer\\Release\\Private CSGO.dll";

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
    if(currentItem != nullptr)
        currentItem->setText(ui->textEdit->toPlainText());
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
    delete[] exeFileAndPath;
    stringRefrences.clear();
    ui->listWidget->clear();
    currentItem = nullptr;
    ui->textEdit->setText("");
    free(fileData);

    QString getFile = QFileDialog::getOpenFileName();
    if(getFile.isEmpty())
        return;

    QByteArray inUtf8 = getFile.toUtf8();
    exeFileAndPath = new char[inUtf8.size()+1]();
    memcpy(exeFileAndPath,(void*)inUtf8.data(),inUtf8.size());

    loadInStrings(exeFileAndPath);
}






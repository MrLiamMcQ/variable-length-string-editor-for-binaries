/********************************************************************************
** Form generated from reading UI file 'instructionsmenu.ui'
**
** Created by: Qt User Interface Compiler version 5.11.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_INSTRUCTIONSMENU_H
#define UI_INSTRUCTIONSMENU_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QLabel>

QT_BEGIN_NAMESPACE

class Ui_instructionsMenu
{
public:
    QDialogButtonBox *buttonBox;
    QLabel *label;
    QLabel *label_2;
    QLabel *label_3;
    QLabel *label_4;
    QLabel *label_5;
    QLabel *label_6;
    QLabel *label_7;
    QLabel *label_8;
    QLabel *label_9;
    QLabel *label_10;

    void setupUi(QDialog *instructionsMenu)
    {
        if (instructionsMenu->objectName().isEmpty())
            instructionsMenu->setObjectName(QStringLiteral("instructionsMenu"));
        instructionsMenu->resize(640, 480);
        buttonBox = new QDialogButtonBox(instructionsMenu);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setGeometry(QRect(460, 440, 171, 32));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        label = new QLabel(instructionsMenu);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(20, 10, 221, 31));
        label_2 = new QLabel(instructionsMenu);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(20, 50, 481, 101));
        label_3 = new QLabel(instructionsMenu);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setGeometry(QRect(20, 160, 231, 21));
        label_4 = new QLabel(instructionsMenu);
        label_4->setObjectName(QStringLiteral("label_4"));
        label_4->setGeometry(QRect(290, 20, 81, 16));
        label_5 = new QLabel(instructionsMenu);
        label_5->setObjectName(QStringLiteral("label_5"));
        label_5->setGeometry(QRect(290, 40, 311, 101));
        label_6 = new QLabel(instructionsMenu);
        label_6->setObjectName(QStringLiteral("label_6"));
        label_6->setGeometry(QRect(20, 190, 361, 151));
        label_7 = new QLabel(instructionsMenu);
        label_7->setObjectName(QStringLiteral("label_7"));
        label_7->setGeometry(QRect(20, 350, 171, 31));
        label_8 = new QLabel(instructionsMenu);
        label_8->setObjectName(QStringLiteral("label_8"));
        label_8->setGeometry(QRect(20, 390, 381, 71));
        label_9 = new QLabel(instructionsMenu);
        label_9->setObjectName(QStringLiteral("label_9"));
        label_9->setGeometry(QRect(400, 200, 271, 71));
        label_10 = new QLabel(instructionsMenu);
        label_10->setObjectName(QStringLiteral("label_10"));
        label_10->setGeometry(QRect(400, 270, 191, 51));

        retranslateUi(instructionsMenu);
        QObject::connect(buttonBox, SIGNAL(accepted()), instructionsMenu, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), instructionsMenu, SLOT(reject()));

        QMetaObject::connectSlotsByName(instructionsMenu);
    } // setupUi

    void retranslateUi(QDialog *instructionsMenu)
    {
        instructionsMenu->setWindowTitle(QApplication::translate("instructionsMenu", "instructions", nullptr));
        label->setText(QApplication::translate("instructionsMenu", "<html><head/><body><p><span style=\" font-size:10pt;\">How to edit strings in an application:</span></p></body></html>", nullptr));
        label_2->setText(QApplication::translate("instructionsMenu", "<html><head/><body><p>1. Press the open button and select a file.</p><p>2. Click on a string displayed in the left hand box.</p><p>3. Edit that string in the righ hand editing pannel.</p><p>4. Click &quot;Make the change to the exe&quot;</p></body></html>", nullptr));
        label_3->setText(QApplication::translate("instructionsMenu", "<html><head/><body><p><span style=\" font-size:10pt;\">Cant find the string?</span></p></body></html>", nullptr));
        label_4->setText(QApplication::translate("instructionsMenu", "<html><head/><body><p><span style=\" font-size:10pt;\">Warning:</span></p></body></html>", nullptr));
        label_5->setText(QApplication::translate("instructionsMenu", "<html><head/><body><p>The change to the exe is a lossy process. </p><p>The lost data is stored in a save file.</p><p>The saved file is the name of the application and &quot;.txt&quot;</p></body></html>", nullptr));
        label_6->setText(QApplication::translate("instructionsMenu", "<html><head/><body><p>the string may not be contained on the exe itself.</p><p>the file may have its strings section commpressed or it might be packed or obfuscated.</p><p>it may be an image and not actualy text.</p><p>it may be stored in the code section. this verstion cant currently find that.</p><p>you may have settings that stop your string from showing up.</p><p>some strings are stored in a the resources so cannot be edited here as i dont know how they are referenced in macheane code.</p></body></html>", nullptr));
        label_7->setText(QApplication::translate("instructionsMenu", "<html><head/><body><p><span style=\" font-size:10pt;\">Problem editing the string?</span></p></body></html>", nullptr));
        label_8->setText(QApplication::translate("instructionsMenu", "<html><head/><body><p>some strings have a hard coded lenght done by the compiler </p><p>for certain functions so you wont be able to change the lenght of the string.</p><p>some programs are signed and so the signature wont match when edited</p></body></html>", nullptr));
        label_9->setText(QApplication::translate("instructionsMenu", "<html><head/><body><p><span style=\" color:#00aa00;\">Green means a reference to the string has been </span></p><p><span style=\" color:#00aa00;\">found in the code and so can be edited</span></p></body></html>", nullptr));
        label_10->setText(QApplication::translate("instructionsMenu", "<html><head/><body><p><span style=\" color:#aa0000;\">red means no references where found</span></p><p><span style=\" color:#aa0000;\"> and so string cannot be edited</span></p></body></html>", nullptr));
    } // retranslateUi

};

namespace Ui {
    class instructionsMenu: public Ui_instructionsMenu {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_INSTRUCTIONSMENU_H

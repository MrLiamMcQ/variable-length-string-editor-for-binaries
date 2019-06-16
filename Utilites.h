#pragma once
#include "pch.h"
#include "Utilites.h"

struct stringReferenceClass {
    bool originWasWchar;
    DWORD addressOfString;
    DWORD referenceOfString;
    std::wstring string;
    std::vector<DWORD> stringReferenceLocations;

    std::wstring changedString;
    DWORD changedStringAddress;

    stringReferenceClass() {}
    stringReferenceClass(DWORD addressOfString, DWORD referenceOfString, std::wstring string,bool originWasWchar) {
        this->addressOfString = addressOfString;
        this->referenceOfString = referenceOfString;
        this->string = string;
        this->originWasWchar = originWasWchar;
    }

    void editString(char*& data);
    void undoStringEdit(char*& data);
};
// reverse bytes ( location in file	of string   -	location in file of header		+  image base   +      RVA)
DWORD getVirtualAddressFromPyisical(DWORD locOfDataPhysical, DWORD headerLocPhysical, DWORD imageBase, DWORD RVA);

bool isLetter(char letter);

int numberOfSymbols(const wchar_t* string);

void findStringReference(char*& fileData, std::vector<stringReferenceClass>& referencesToFind, int beginAddress, int amountToSerch, std::mutex& mainMutex);

std::string convStr(std::string string);
std::string convStrBack(std::string string);

void saveData(std::string fileName, std::vector<stringReferenceClass>& stringRefrences);
std::vector<stringReferenceClass> loadData(std::string fileName);

std::vector<stringReferenceClass> getStringData(char*& fileData);

#include "Utilites.h"


bool isLetter(wchar_t letter) {
	if ((int)letter > 31 && (int)letter < 127 || letter== '\n')
		return true;
	return false;
}

int numberOfSymbols(const wchar_t* string) {
    int noOfSymbols = wcslen(string);
    for (int i = 0; i < wcslen(string) - 1; i++) {
		if ((int)string[i] > 64 && (int)string[i] < 91) {
			noOfSymbols--;
		}
		else if ((int)string[i] > 96 && (int)string[i] < 123) {
			noOfSymbols--;
		}
	}
	return noOfSymbols;
}

void stringReferenceClass::editString(char*& data) {
	for (auto a : this->stringReferenceLocations) {
		data[a] = (this->changedStringAddress >> 24);
		data[a+1] = (this->changedStringAddress >> 16);
		data[a + 2] = (this->changedStringAddress >> 8);
		data[a + 3] = (this->changedStringAddress);
	}
}

void stringReferenceClass::undoStringEdit(char*& data) {
	for (auto a : this->stringReferenceLocations) {
		data[a] = (this->referenceOfString >> 24);
		data[a + 1] = (this->referenceOfString >> 16);
		data[a + 2] = (this->referenceOfString >> 8);
		data[a + 3] = (this->referenceOfString);
	}
}

//
// 2800 data's positoin in file
//
// 400000 - image base
// 006000 - RVA of section header that data is held in
//      A - data position relative to section's start
//
// 40600A - what that that position in file is refered to as by assembly

DWORD getVirtualAddressFromPyisical(DWORD locOfDataPhysical, DWORD headerLocPhysical, DWORD imageBase, DWORD RVA) {
	DWORD relativeAddressFromHeader = (locOfDataPhysical - headerLocPhysical);
	DWORD RETURNTHING = _byteswap_ulong(relativeAddressFromHeader + imageBase + RVA);
	return RETURNTHING;
}

//slow method of getting string references faster would be dissasembler
//then geting all the addresses and cheaking aganst list of string addresses

void findStringReference(char*& fileData, std::vector<stringReferenceClass>& referencesToFind, int beginAddress, int amountToSerch, std::mutex& mainMutex) {
	for (int i = beginAddress; i < (beginAddress + amountToSerch); i += 1) {
		for (auto& j : referencesToFind) {
			if (j.referenceOfString == ((fileData[i] << 24) | (fileData[i + 1] << 16) | (fileData[i + 2] << 8) | fileData[i + 3])) {
				//std::cout << "found " << j.referenceOfString << " " << j.string.c_str() << " " << i << std::endl;
				mainMutex.lock();
				j.stringReferenceLocations.push_back(i);
				mainMutex.unlock();
			}
		}
	}
}

std::string convStr(std::string string) {
	int strLen = string.size();
	std::string literalNewLine = "\\n";
	for (int i = 0; i < string.size(); i++) {
		if (string[i] == '\n') {
			string.replace(i, 2, literalNewLine);
			strLen = string.size();
		}
	}
	return string;
}

std::string convStrBack(std::string string) {
	int strLen = string.size();
	for (int i = 0; i < string.size(); i++) {
		if (string[i] == '\\' && string[i + 1] == 'n') {
			string[i] = '\n';
			string[i + 1] = ' ';
		}
	}
	return string;
}

void saveData(std::string fileName, std::vector<stringReferenceClass>& stringRefrences) {
    FILE* saveFile;
    fopen_s(&saveFile, fileName.c_str(), "wb");
    wchar_t endOfFileMarker = 0xAAAA;

    for (int i = 0; i < stringRefrences.size(); i++) {
        fwrite(&stringRefrences[i].originWasWchar, 1, 1, saveFile);
        fwrite(&stringRefrences[i].addressOfString, 1, 4, saveFile);//DWORD
        fwrite(&stringRefrences[i].changedStringAddress, 1, 4, saveFile);//DWORD
        fwrite(&stringRefrences[i].referenceOfString, 1, 4, saveFile);//DWORD

        for (int j = 0; j < stringRefrences[i].stringReferenceLocations.size(); j++) {
            fwrite(&stringRefrences[i].stringReferenceLocations[j], 1, 4, saveFile);//DWORD
        }

        DWORD markerEndOfDwords = 0x99889988;
        fwrite(&markerEndOfDwords, 1, 4, saveFile);//DWORD marker for start of strings

        fwrite(stringRefrences[i].changedString.c_str(), 2, stringRefrences[i].changedString.size(), saveFile);// removed + 1
        wchar_t endOfString = 0x9090;
        fwrite(&endOfString, 1, 2, saveFile); // double nop seperated strings 9090
        fwrite(stringRefrences[i].string.c_str(), 2, stringRefrences[i].string.size(), saveFile);// removed + 1
        wchar_t endOfClassMarker = 0xCCCC;
        if (i == stringRefrences.size() - 1)
            endOfClassMarker = endOfFileMarker;
        fwrite(&endOfClassMarker, 1, 2, saveFile); // 0xCC marker for end of class 0xAA marker end of file
    }
    fclose(saveFile);
}

std::vector<stringReferenceClass> loadData(std::string fileName) {
    std::vector<stringReferenceClass> returnValue;

    FILE* loadFile;
    fopen_s(&loadFile, fileName.c_str(), "rb");
    bool fileEndReached = false;

    do {
        stringReferenceClass newRefClass;

        bool wasWchar;
        DWORD addressOfString;
        DWORD changedStringAddress;
        DWORD referenceOfString;
        fread(&wasWchar, 1, 1, loadFile);
        fread(&addressOfString, 1, 4, loadFile);//DWORD
        fread(&changedStringAddress, 1, 4, loadFile);//DWORD
        fread(&referenceOfString, 1, 4, loadFile);//DWORD

        DWORD currentDWORD;
        std::vector<DWORD> stringReferenceLocations;
        while (1) {
            fread(&currentDWORD, 1, 4, loadFile);
            if (currentDWORD != 0x99889988)
                stringReferenceLocations.push_back(currentDWORD);
            else break;
        }

        std::wstring changedString = L"";
        std::wstring string = L"";
        bool firstString = true;
        while (1) {
            wchar_t currentChar;
            fread(&currentChar, 1, 2, loadFile);
            if (currentChar != (wchar_t)0x9090 && firstString)
                changedString.push_back(currentChar);
            else if (currentChar != (wchar_t)0xCCCC && currentChar != (wchar_t)0xAAAA) {
                if (currentChar != (wchar_t)0x9090)
                    string.push_back(currentChar);
                firstString = false;
            }
            else if (currentChar == (wchar_t)0xAAAA) {
                fileEndReached = true;
                break;
            }
            else break;
        }

        newRefClass.originWasWchar = wasWchar;
        newRefClass.addressOfString = addressOfString;
        newRefClass.changedString = changedString;
        newRefClass.changedStringAddress = changedStringAddress;
        newRefClass.referenceOfString = referenceOfString;
        newRefClass.string = string;
        newRefClass.stringReferenceLocations = stringReferenceLocations;

        returnValue.push_back(newRefClass);

    } while (!fileEndReached);


    fclose(loadFile);

    return returnValue;
}


std::vector<stringReferenceClass> getStringData(char*& fileData) {
    std::vector<stringReferenceClass> stringRefrences;// test reading and writeing file

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

        if (strcmp(sectionName, ".text") == 0) {
            codeSectionAddress = addresOfSectionData;
            sizeOfCodeSection = sizeOfSection;
        }

        if (strcmp(sectionName, ".rdata") != 0)
            continue;

        std::wstring sString;
        std::wstring charString;
        for (int i = addresOfSectionData; i < addresOfSectionData + sizeOfSection; i++) {
            // find char string put into charString
            wchar_t wChar = (wchar_t)(fileData[i]);
            if (isLetter(wChar))
                charString += wChar;
            else if(charString.size()>3){
                if (charString.find(L"RSDS")==0)
                    break;
                DWORD addressOfString = (i - charString.size());
                DWORD addres = getVirtualAddressFromPyisical(addressOfString, addresOfSectionData, OH->ImageBase, SH[currentIndexOfSections].VirtualAddress);
                stringRefrences.emplace_back(addressOfString, addres, charString,false);
                charString.clear();
            }
            else if (wChar == NULL)
                charString.clear();

            // find wchars put into sString
            bool isNull = false;
            wchar_t curentWchar = (wchar_t)(fileData[i] << 8) + fileData[i + 1];
            if ((int)fileData[i] == 0 && (int)fileData[i + 1] == 0)
                isNull = true;
            if (isLetter(curentWchar)) {
                sString += curentWchar;
            }
            else if (sString.size() > 3 && isNull) {

                int symbolCount = numberOfSymbols(sString.c_str());
                float persentageSymbols = (float)((float)symbolCount / sString.size());

                if (persentageSymbols >= 0.5) {
                    sString.clear();
                    continue;
                }

                DWORD addressOfString = (i+1 - (sString.size()*2));
                DWORD addres = getVirtualAddressFromPyisical(addressOfString, addresOfSectionData, OH->ImageBase, SH[currentIndexOfSections].VirtualAddress);
                stringRefrences.emplace_back(addressOfString, addres, sString,true);

                sString.clear();
            }
            else if (isNull)
                sString.clear();
        }

    }

    //multithreded find strings references in code
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

#include "Utilites.h"


bool isLetter(char letter) {
	if ((int)letter > 31 && (int)letter < 127 || letter== '\n')
		return true;
	return false;
}

int numberOfSymbols(const char* string) {
	int noOfSymbols = strlen(string);
	for (int i = 0; i < strlen(string) - 1; i++) {
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
	char endOfFileMarker = 0xAA;

	for (int i = 0; i < stringRefrences.size(); i++) {
		fwrite(&stringRefrences[i].addressOfString, 1, 4, saveFile);//DWORD
		fwrite(&stringRefrences[i].changedStringAddress, 1, 4, saveFile);//DWORD
		fwrite(&stringRefrences[i].referenceOfString, 1, 4, saveFile);//DWORD

		for (int j = 0; j < stringRefrences[i].stringReferenceLocations.size(); j++) {
			fwrite(&stringRefrences[i].stringReferenceLocations[j], 1, 4, saveFile);//DWORD
		}

		DWORD markerEndOfDwords = 0x99889988;
		fwrite(&markerEndOfDwords, 1, 4, saveFile);//DWORD marker for start of strings

		fwrite(stringRefrences[i].changedString.c_str(), 1, stringRefrences[i].changedString.size() + 1, saveFile);
		DWORD endOfString = 0x90;
		fwrite(&endOfString, 1, 1, saveFile); // nop seperated strings 90
		fwrite(stringRefrences[i].string.c_str(), 1, stringRefrences[i].string.size() + 1, saveFile);
		DWORD endOfClassMarker = 0xCC;
		if (i == stringRefrences.size() - 1)
			endOfClassMarker = endOfFileMarker;
		fwrite(&endOfClassMarker, 1, 1, saveFile); // 0xCC marker for end of class 0xAA marker end of file
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

		DWORD addressOfString;
		DWORD changedStringAddress;
		DWORD referenceOfString;
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

		std::string changedString;
		std::string string;
		bool firstString = true;
		while (1) {
			char currentChar;
			fread(&currentChar, 1, 1, loadFile);
			if (currentChar != (char)0x90 && firstString)
				changedString.push_back(currentChar);
			else if (currentChar != (char)0xCC && currentChar != (char)0xAA) {
                if (currentChar != (char)0x90)
					string.push_back(currentChar);
				firstString = false;
			}
			else if (currentChar == (char)0xAA) {
				fileEndReached = true;
				break;
			}
			else break;
		}

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

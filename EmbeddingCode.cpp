#include "pch.h"
#include "EmbeddingCode.h"

DWORD align(DWORD size, DWORD align, DWORD addr) {
	if (!(size % align))
		return addr + size;
	return addr + (size / align + 1) * align;
}

std::vector<DWORD> writeNewSection(char*& programData, int& dataSize, DWORD uniqueIdentifyer, const char* nameOfSection, DWORD charicteristecs, std::vector<stringReferenceClass>& stringsToEmbed) {

	PIMAGE_DOS_HEADER pDosHeader_setup = (PIMAGE_DOS_HEADER)programData;
	PIMAGE_OPTIONAL_HEADER OH_setup = (PIMAGE_OPTIONAL_HEADER)(programData + pDosHeader_setup->e_lfanew + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER));

	// get size of new section and extend size appropriately

	int sizeOfNewSection = 20;
	std::vector<DWORD> addressesOfStrings;

	for (auto& string : stringsToEmbed) {
		if (string.changedString == "")
			continue;

		sizeOfNewSection += (string.changedString.size()+1);
	};

	sizeOfNewSection = align(sizeOfNewSection, OH_setup->FileAlignment, 0);

	char* newProgramData = new char[dataSize+sizeOfNewSection]();
	memcpy(newProgramData,programData, dataSize);

	delete[] programData;
	programData = newProgramData;

	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)programData;
	PIMAGE_FILE_HEADER FH = (PIMAGE_FILE_HEADER)(programData + pDosHeader->e_lfanew + sizeof(DWORD));
	PIMAGE_OPTIONAL_HEADER OH = (PIMAGE_OPTIONAL_HEADER)(programData + pDosHeader->e_lfanew + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER));
	PIMAGE_SECTION_HEADER SH = (PIMAGE_SECTION_HEADER)(programData + pDosHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS));

	// write new section's header into data

	ZeroMemory(&SH[FH->NumberOfSections], sizeof(IMAGE_SECTION_HEADER));
	CopyMemory(&SH[FH->NumberOfSections].Name, nameOfSection, 8);

	SH[FH->NumberOfSections].Misc.VirtualSize = align(sizeOfNewSection, OH->SectionAlignment, 0);
	SH[FH->NumberOfSections].VirtualAddress = align(SH[FH->NumberOfSections - 1].Misc.VirtualSize, OH->SectionAlignment, SH[FH->NumberOfSections - 1].VirtualAddress);

	// compiler makes theses section header values all zero later on without warning or reason so have to save the value for use
	DWORD savedRVA = SH[FH->NumberOfSections].VirtualAddress;

	SH[FH->NumberOfSections].SizeOfRawData = align(sizeOfNewSection, OH->FileAlignment, 0);
	SH[FH->NumberOfSections].PointerToRawData = dataSize;
	SH[FH->NumberOfSections].Characteristics = charicteristecs;

	OH->SizeOfImage = SH[FH->NumberOfSections].VirtualAddress + SH[FH->NumberOfSections].Misc.VirtualSize;
	FH->NumberOfSections += 1;

	// write data into new sections data

	for (int i = 0; i < 4; i++) {
		newProgramData[(dataSize)+i] = (uniqueIdentifyer >> (i * 8));
	}

	DWORD currentMemLoc = 10 + dataSize;
	for (auto& string : stringsToEmbed) {

		if (string.changedString == "")
			continue;

		for (int i = 0; i < string.changedString.size(); i++) {
			programData[currentMemLoc + i] = string.changedString[i];
			if (i == 0)						
				string.changedStringAddress = getVirtualAddressFromPyisical(currentMemLoc, dataSize, OH->ImageBase, savedRVA);
		}
		currentMemLoc += (string.changedString.size() + 1);
	};

	dataSize += sizeOfNewSection;

	return addressesOfStrings;
}

void deleateLastSection(char*& programData, int& dataSize) {
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)programData;
	PIMAGE_FILE_HEADER FH = (PIMAGE_FILE_HEADER)(programData + pDosHeader->e_lfanew + sizeof(DWORD));
	PIMAGE_OPTIONAL_HEADER OH = (PIMAGE_OPTIONAL_HEADER)(programData + pDosHeader->e_lfanew + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER));
	PIMAGE_SECTION_HEADER SH = (PIMAGE_SECTION_HEADER)(programData + pDosHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS));

	int lastSecIndex = FH->NumberOfSections - 1;
	OH->SizeOfImage -= SH[lastSecIndex].Misc.VirtualSize;
	FH->NumberOfSections -= 1;

	int sizeOfNewFile = dataSize - SH[lastSecIndex].SizeOfRawData;
	int saved_e_lfanew = pDosHeader->e_lfanew;

	char* newProgData = new char[sizeOfNewFile];
	memcpy(newProgData, programData, sizeOfNewFile);
	delete[] programData;
	programData = newProgData;
	dataSize = sizeOfNewFile;

	void* addressOfLastSectionHeader = (programData + saved_e_lfanew + sizeof(IMAGE_NT_HEADERS)) + (lastSecIndex * sizeof(IMAGE_SECTION_HEADER));

	ZeroMemory(addressOfLastSectionHeader, sizeof(IMAGE_SECTION_HEADER));
}

bool openSectionHeader(char*& programData, int& dataSize, DWORD uniqueIdentifyer, const char* nameOfSection, DWORD charicteristecs, std::vector<stringReferenceClass>& stringsToEmbed) {

	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)programData;
	PIMAGE_FILE_HEADER FH = (PIMAGE_FILE_HEADER)(programData + pDosHeader->e_lfanew + sizeof(DWORD));
	PIMAGE_OPTIONAL_HEADER OH = (PIMAGE_OPTIONAL_HEADER)(programData + pDosHeader->e_lfanew + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER));
	PIMAGE_SECTION_HEADER SH = (PIMAGE_SECTION_HEADER)(programData + pDosHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS));

	// not working should not be finding unique identifyer

	bool foundItem = false;
	for (int i = 0; i < FH->NumberOfSections; i++) {

		int startOfSectionHeader = SH[i].PointerToRawData;
		int numCorrect = 0;
		for (int j = 0; j < 4; j++) {
			if (programData[(startOfSectionHeader)+j] == (char)(uniqueIdentifyer >> (j * 8)))
				numCorrect++;
		}
		if (numCorrect != 4)
			continue;

		foundItem = true;
		// found section header it already exists
		if (i != FH->NumberOfSections - 1)
			continue;

		deleateLastSection(programData, dataSize);
		writeNewSection(programData, dataSize, uniqueIdentifyer, nameOfSection, charicteristecs, stringsToEmbed);
	}
	if (!foundItem)
		writeNewSection(programData, dataSize, uniqueIdentifyer, nameOfSection, charicteristecs, stringsToEmbed);

	return 0;
}
#pragma once

#include "Utilites.h"

DWORD align(DWORD size, DWORD align, DWORD addr);
std::vector<DWORD> writeNewSection(char*& programData, int& dataSize, DWORD uniqueIdentifyer, const char* nameOfSection, DWORD charicteristecs, std::vector<stringReferenceClass>& stringsToEmbed);
void deleateLastSection(char*& programData, int& dataSize);
bool openSectionHeader(char*& programData, int& dataSize, DWORD uniqueIdentifyer, const char* nameOfSection, DWORD charicteristecs, std::vector<stringReferenceClass>& stringsToEmbed);
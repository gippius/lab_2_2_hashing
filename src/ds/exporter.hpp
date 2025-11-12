#pragma once
#include <string>
class HashTable;

// Выгружает три файла в указанную папку: slots.csv, probes.csv, clusters.csv
bool exportAllCSV(const HashTable& table, const std::string& dirPath);

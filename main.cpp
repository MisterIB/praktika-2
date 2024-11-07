#include <iostream>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <filesystem>
#include <cstdio>
#include <algorithm>
#include <arpa/inet.h>
#include <unistd.h>
#include <mutex>
#include <thread>

#include "HashTable.h"
#include "List.h"

using namespace std;

struct Configuration {
	string name;
	int32_t tuples_limits = 1;
};

Configuration configuration;

void checkTheFileOpening(ifstream& file) {
	if (!file.is_open()) throw runtime_error("Error open file");
}

void checkTheFileOpening(ofstream& file) {
	if (!file.is_open()) throw runtime_error("Error open file");
}

void chekTheFileUnlock(const string& tableName) {
	ifstream fileLockTable(configuration.name + '/' + tableName + '/' + tableName + "_lock");//Linux изменить
	checkTheFileOpening(fileLockTable);
	int32_t statusFile;
	fileLockTable >> statusFile;
	if (statusFile == 1) throw runtime_error("Table locking");
	fileLockTable.close();
}

void createMainDirectory(string& name, bool& databaseCreating) {
	if (mkdir(name.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) databaseCreating = true;
}

string readName(ifstream& file, bool& databaseCreating) {
	unsigned char letter;
	string name = "";
	file >> letter;
	if (letter == '"') file >> letter;
	while (letter != '"') {
		name += letter;
		file >> letter;
	}
	createMainDirectory(name, databaseCreating);
	return name;
}

int32_t readTuplesLimits(ifstream& file) {
	char figure;
	string number;
	file >> figure;
	while (figure != ',') {
		number += figure;
		file >> figure;
	}
	return stoi(number);
}

string readNameTable(ifstream& file, unsigned char& letter) {
	file >> letter;
	string name;
	for (int32_t countLetter = 0; letter != '"'; countLetter++) {
		name += letter;
		file >> letter;
		if (countLetter > 25) throw runtime_error("incorrect table name");
	}
	return name;
}

void WritingToFileNamesColumns(ifstream& file, ofstream& fileTable) {
	unsigned char letter;
	file >> letter;
	while (true) {
		string name;
		while (letter != '"') {
			name += letter;
			file >> letter;
		}
		fileTable << name;
		file >> letter;
		if (letter == ',') {
			file >> letter;
			file >> letter;
			fileTable << ", ";
		}
		else break;
	}
}

void readNamesColumns(ifstream& file, ofstream& fileTable) {
	unsigned char letter;
	file >> letter;
	if (letter == '"') {
		file >> letter;
		while (true) {
			string name;
			while (letter != '"') {
				name += letter;
				file >> letter;
			}
			fileTable << name;
			file >> letter;
			if (letter == ',') {
				file >> letter;
				file >> letter;
				fileTable << ", ";
			}
			else break;
		}
	}
}

void readСolumnsOfTable(ifstream& file, ofstream& fileTable, unsigned char& letter) {
	if (letter == '"') file >> letter;
	if (letter == ':') file >> letter;
	else throw runtime_error("Incorrect data in the \"structure\"");
	readNamesColumns(file, fileTable);
}

void createFileLockTable(const string& path, const string& nameTable) {
	ofstream fileLockTable(path + '/' + nameTable + "_lock");
	if (fileLockTable.is_open()) fileLockTable << 0;
	else throw runtime_error("Error create file <Table name>_lock");
	fileLockTable.close();
}

void createFileForPrimaryKey(const string& path, const string& nameTable) {
	ofstream filePrimaryKey(path + '/' + nameTable + "_pk_sequence");
	if (filePrimaryKey.is_open()) filePrimaryKey << 1;
	else throw runtime_error("Error create file <Table name>_pk_sequence");
	filePrimaryKey.close();
}

void createFilesTable(const string& path, const string& nameTable) {
	if (mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != -1) {
	    createFileLockTable(path, nameTable);
	    createFileForPrimaryKey(path, nameTable);
    }
}

void readStructure(ifstream& file) {
	unsigned char letter;
	file >> letter;
	if (letter == '{') file >> letter;
	if (letter == '"') {
		for (int32_t i = 1; true; i++) {
			string nameTable = readNameTable(file, letter);
			string path = configuration.name + '/' + nameTable;
			createFilesTable(path, nameTable);
			ofstream fileTable(path + '/' + "1" + ".csv");
			if (fileTable.is_open()) {
				readСolumnsOfTable(file, fileTable, letter);
			}
			else throw runtime_error("Error create file");
			file >> letter;
			if (letter == ',') file >> letter;
			else break;
			fileTable.close();
		}
	}
}

void readingConfiguration(ifstream& file) {
	string inputStr;
    bool databaseCreating = false;
	while (file >> inputStr) {
		if (inputStr == "\"name\":") configuration.name = readName(file, databaseCreating);
		if (inputStr == "\"tuples_limit\":") configuration.tuples_limits = readTuplesLimits(file);
		if (inputStr == "\"structure\":" and !databaseCreating) readStructure(file);
	}
}

void createDataBase() {
	ifstream file("schema.json");
	if (file.is_open()) {
		readingConfiguration(file);
	}
	else throw runtime_error("Error file open");
	file.close();
}

bool checkingTableName(string& tableName) {
	struct stat md;
	string dir = configuration.name + '/' + tableName;
	if (stat(dir.c_str(), &md) == 0) return true;
	else throw runtime_error("Invalid table name");
}

void lockingTheTable(const string& tableName) {
	ofstream fileLockTable(configuration.name + '/' + tableName + '/' + tableName + "_lock");
	checkTheFileOpening(fileLockTable);
	fileLockTable << 1;
	fileLockTable.close();
}

void unlockingTheTable(const string& tableName) {
	ofstream fileLockTable(configuration.name + '/' + tableName + '/' + tableName + "_lock");
	checkTheFileOpening(fileLockTable);
	fileLockTable << 0;
	fileLockTable.close();
}

int32_t amountOfFilesInDir(const string& tableName) {
	auto dirIter = filesystem::directory_iterator(configuration.name + '/' + tableName);
	int32_t fileCount = count_if(begin(dirIter), end(dirIter), [](auto& entry) { return entry.is_regular_file(); });
	return fileCount;
}

void updateFilePrimaryKey(const string& tableName, int32_t primaryKey) {
	ofstream filePrimaryKey(configuration.name + '/' + tableName + '/' + tableName + "_pk_sequence");
	checkTheFileOpening(filePrimaryKey);
	primaryKey++;
	filePrimaryKey << primaryKey;
	filePrimaryKey.close();
}

int32_t readPrimaryKey(const string& tableName) {
	ifstream filePrimaryKey(configuration.name + '/' + tableName + '/' + tableName + "_pk_sequence");
	int32_t primaryKey;
	checkTheFileOpening(filePrimaryKey);
	filePrimaryKey >> primaryKey;
	filePrimaryKey.close();
	return primaryKey;
}

int32_t checkingTuplesLimit(const string& tableName) {
	int32_t primaryKey = readPrimaryKey(tableName);
	int32_t amountOfTableFiles = amountOfFilesInDir(tableName) - 2;
	if (primaryKey / configuration.tuples_limits + 1 != amountOfTableFiles) amountOfTableFiles++;
	updateFilePrimaryKey(tableName, primaryKey);
	return amountOfTableFiles;
}

void addValuesToVector(unsigned char& character, List<string>& valuesForNewRow, istringstream& inputCommandStream) {
	string value;
	for (int32_t i = 0; character != ')'; i++) {
		inputCommandStream >> character;
		if (i == 0 or character == '\'') continue;
		if (character == ',') {
			valuesForNewRow.push_back(value);
			i = 0;
			value = "";
			inputCommandStream >> character;
			inputCommandStream >> character;
		}
		if (character == ')') valuesForNewRow.push_back(value);
		value += character;
	}
}

void writeRowToFile(const string& nameOfTableFile, List<string> valuesForNewRow, int32_t amountOfColumns) {
	ofstream tableFile(configuration.name + '/' + nameOfTableFile, ios_base::app);
	checkTheFileOpening(tableFile);
	if (valuesForNewRow.size > amountOfColumns) throw runtime_error("Incorrect amount of input data");
	for (int32_t i = 0; i < amountOfColumns; i++) {
		if (i == 0) tableFile << '\n' << valuesForNewRow.get(i) << ", ";
		else if (i + 1 != valuesForNewRow.size and i < valuesForNewRow.size) tableFile << valuesForNewRow.get(i) << ", ";
		else if (i + 1 == amountOfColumns and valuesForNewRow.size != amountOfColumns) tableFile << "NULL";
		else if (i >= valuesForNewRow.size) tableFile << "NULL" << ", ";
		else if (i + 1 >= valuesForNewRow.size and valuesForNewRow.size != amountOfColumns) tableFile << valuesForNewRow.get(i) << ", ";
		else tableFile << valuesForNewRow.get(i);
	}
	tableFile.close();
}

void readInputValues(const string& nameOfTableFile, int32_t amountOfColumns, istringstream& inputCommandStream) {
	List<string> valuesForNewRow;
	unsigned char character;
	inputCommandStream >> character;
	if (character == '(') {
		addValuesToVector(character, valuesForNewRow, inputCommandStream);
	}
	writeRowToFile(nameOfTableFile, valuesForNewRow, amountOfColumns);
}

string readingTableName(istringstream& inputCommandStream) {
	string tableName;
	inputCommandStream >> tableName;
	if (checkingTableName(tableName)) return tableName;
}

int32_t calculateAmountOfColumns(const string& tableName) {
	string nameOfTableFile = configuration.name + '/' + tableName + '/' + "1.csv";
	ifstream fileTable(nameOfTableFile);
	checkTheFileOpening(fileTable);
	string firstStr;
	getline(fileTable, firstStr);
	istringstream firstStrStream(firstStr);
	string column;
	int32_t amountOfColumns = 0;
	while (getline(firstStrStream, column, ' ')) amountOfColumns++;
	fileTable.close();
	return amountOfColumns;
}

void insertingIntoTable(istringstream& inputCommandStream) {
	string tableName = readingTableName(inputCommandStream);
	chekTheFileUnlock(tableName);
	lockingTheTable(tableName);
	int32_t amountOfTableFiles = checkingTuplesLimit(tableName);
	string nameOfTableFile = tableName + '/' + to_string(amountOfTableFiles) + ".csv";
	int32_t amountOfColumns = calculateAmountOfColumns(tableName);
	string command;
	inputCommandStream >> command;
	if (command == "VALUES") readInputValues(nameOfTableFile, amountOfColumns, inputCommandStream);
	unlockingTheTable(tableName);
}

void commandInsertInto(istringstream& inputCommandStream) {
	string command;
	inputCommandStream >> command;
	if (command == "INTO") insertingIntoTable(inputCommandStream);
	else throw runtime_error("Incorrect command");
}

List<string> readLogicalExpression(string str) {
	List<string> logicalExpression;
	string element;
	for (char character : str) {
		if (character == ' ' and element != "") {
			logicalExpression.push_back(element);
			element = "";
		}
		else if (character != ' ') element += character;
	}
	logicalExpression.push_back(element);
	return logicalExpression;
}

void readTableAndColumnName(string& tableName,  string& columnName, string element) {
	string name;
	for (char character : element) {
		if (character == '.') {
			tableName = name;
			name = "";
		}
		if (character != '.') name += character;
		
	}
	columnName = name;
	checkingTableName(tableName);
}

int32_t chekingColumnName(ifstream& fileTable, const string& columnName) {
	unsigned char character;
	string column, row;
	int32_t columnNumber = 1;
	getline(fileTable, row);
	istringstream rowStream(row);
	while (getline(rowStream, column, ' ')) {
		if (column[column.size() - 1] == ',') column.erase(column.size() - 1, 1);
		if (column == columnName) return columnNumber;
		columnNumber++;
	}
	throw runtime_error("Incorrect column name");
}

string returnElementSearch(ifstream& fileTable, int32_t numberColumn, string& str) {
	string column;
	int32_t i = 1;
	for (unsigned char character : str) {
		if (character == ',' and i == numberColumn) {
			return column;
		}
		else if (character == ',') {
			column = "";
			i++;
		}
		if (character != ',') column += character;
	}
}

string elementSearch(ifstream& fileTable, int32_t primaryKey, const string& columnName) {
	string str;
	int32_t numberColumn;
	for (int32_t i = 1; i <= primaryKey; i++) {	
		if (i == 1) {
			numberColumn = chekingColumnName(fileTable, columnName);
			getline(fileTable, str);
		}
		else if (primaryKey == 2) return returnElementSearch(fileTable, numberColumn, str);
		else if (i == 2) getline(fileTable, str);
		else if (i == primaryKey) return returnElementSearch(fileTable, numberColumn, str);
		if (i > 2) getline(fileTable, str);
	}
}

void replaceElementData(string& element, int32_t primaryKey){
	string tableName, columnName;
	readTableAndColumnName(tableName, columnName, element);
	int32_t numberFile = primaryKey / configuration.tuples_limits + 1;
	ifstream fileTable(configuration.name + '/' + tableName + '/' + to_string(numberFile) + ".csv");
	checkTheFileOpening(fileTable);
	element = elementSearch(fileTable, primaryKey, columnName);
	if (element[0] == ' ') element.erase(0, 1);
	fileTable.close();
}

void replaceDataFromTables(List<string>& logicalExpression, int32_t primaryKey) {
	for (int32_t i = 0; i < logicalExpression.size; i++) {
		string& element = logicalExpression.get(i);
		if (element == "=" or element == "AND" or element == "OR" or element[0] == '\'') continue;
		else replaceElementData(element, primaryKey);
	}
}

void replaceStrings(List<string>& logicalExpression) {
	for (int32_t i = 0; i < logicalExpression.size; i++) {
		string& element = logicalExpression.get(i);
		if (element[0] == '\'' and element[element.size() - 1] == '\'') {
			element.erase(0, 1);
            element.erase(element.size() - 1, 1);
		}
	}
}

void makeNewExpression(List<string>& logicalExpression, int32_t j, string value) {
	List<string> newLogicalExpression;
	for (int32_t i = 0; i < logicalExpression.size; i++) {
		if (i == j) newLogicalExpression.push_back(value);
		else if (i == j + 1 or i == j - 1) continue;
		else newLogicalExpression.push_back(logicalExpression.get(0));
	}
	logicalExpression = newLogicalExpression;
}

void replaceExpressionWithComparison(List<string>& logicalExpression) {
	for (int32_t i = 0; i < logicalExpression.size; i++) {
		if (logicalExpression.get(i) == "=" and i != 0 and i + 1 != logicalExpression.size) {
			if (logicalExpression.get(i - 1) == logicalExpression.get(i + 1)) makeNewExpression(logicalExpression, i, "true");
			else makeNewExpression(logicalExpression, i, "false");
		}
	}
}

void replaceTheOrOperator(List<string>& logicalExpression) {
	for (int32_t i = 0; i < logicalExpression.size; i++) {
		if (logicalExpression.get(i) == "OR" and i != 0 and i + 1 != logicalExpression.size) {
			if (logicalExpression.get(i - 1) == "false" and logicalExpression.get(i + 1) == "false")
				makeNewExpression(logicalExpression, i, "false");
			else makeNewExpression(logicalExpression, i, "true");
		}
	}
}

void replaceTheAndOperator(List<string>& logicalExpression) {
	for (int32_t i = 0; i < logicalExpression.size; i++) {
		if (logicalExpression.get(i) == "AND" and i != 0 and i + 1 != logicalExpression.size) {
			if (logicalExpression.get(i - 1) == "true" and logicalExpression.get(i + 1) == "true")
				makeNewExpression(logicalExpression, i, "true");
			else makeNewExpression(logicalExpression, i, "false");
		}
	}
}

bool checkingResultOfSubstitutions(List<string>& logicalExpression) {
	if (logicalExpression.get(0) == "true") return true;
	if (logicalExpression.get(0) == "false") return false;
}

bool chekingCommandWhere(istringstream& inputCommandStream) {
	string inputCommand;
	inputCommandStream >> inputCommand;
	if (inputCommand == "WHERE") return true;
	else return false;
}

bool commandWhere(int32_t primaryKey, List<string> logicalExpression) {
	replaceDataFromTables(logicalExpression, primaryKey);
	replaceStrings(logicalExpression);
	replaceExpressionWithComparison(logicalExpression);
	replaceTheOrOperator(logicalExpression);
	replaceTheAndOperator(logicalExpression);
	bool returnValue = checkingResultOfSubstitutions(logicalExpression);
	return returnValue;
}

void skipRow(ifstream& fileTable) {
	unsigned char character;
	do {
		fileTable >> character;
	} while (character != '\n');
}

void reducePrimaryKey(const string& tableName, int32_t primaryKey) {
	ofstream filePrimaryKey(configuration.name + '/' + tableName + '/' + tableName + "_pk_sequence");
	checkTheFileOpening(filePrimaryKey);
	primaryKey--;
	filePrimaryKey << primaryKey;
	filePrimaryKey.close();
}

void deleteRow(int32_t primaryKey, const string& path) {
	ofstream newFileTable(path + "copy");
	ifstream fileTable(path);
	checkTheFileOpening(fileTable);
	checkTheFileOpening(newFileTable);
	string str;
	int32_t numberColumn = 0;
	while (getline(fileTable, str)) {
		numberColumn++;
		if (numberColumn == primaryKey) continue;
		else {
			newFileTable << str;
			newFileTable << '\n';
		}
	}
	newFileTable.close();
	fileTable.close();
	if (filesystem::remove(path) != 1) throw runtime_error("Error remove file");
	filesystem::rename(path + "copy", path);
}

void changingRows(const string& tableName, const string& path, istringstream& inputCommandStream) {
	int32_t primaryKey = readPrimaryKey(tableName);
	if (!chekingCommandWhere(inputCommandStream)) throw runtime_error("Incorrect input command");
	string str;
	getline(inputCommandStream, str);
	for (int32_t i = primaryKey; i > 1; i--) {
		List<string> logicalExpression = readLogicalExpression(str);
		if (commandWhere(i, logicalExpression)) {
			deleteRow(i, path);
			reducePrimaryKey(tableName, primaryKey);
		}
	}
}

void сhangingFiles(const string& tableName, istringstream& inputCommandStream) {
	int32_t amountOfTableFiles = amountOfFilesInDir(tableName) - 2;
	for (int32_t i = 1; i <= amountOfTableFiles; i++) {
		string path = configuration.name + '/' + tableName + '/' + to_string(i) + ".csv";
		lockingTheTable(tableName);
		changingRows(tableName, path, inputCommandStream);
		unlockingTheTable(tableName);
	}
}

void commandDeleteFrom(istringstream& inputCommandStream) {
	string command;
	inputCommandStream >> command;
	if (command == "FROM") {
		string tableName = readingTableName(inputCommandStream);
		сhangingFiles(tableName, inputCommandStream);
	}
}

void readColumnName(string& columnNames, istringstream& inputCommandStream) {
	string  str = "", tempstr, tableNames, fullStr = "";
	while (str != "FROM") {
		inputCommandStream >> str;
		if (str != "FROM") fullStr += str;
	}
	for (unsigned char character : fullStr) {
		if (character == '.') {
			tableNames += tempstr + ' ';
			tempstr += ".";
		}
		else if (character == ',') {
			columnNames += tempstr + ' ';
			tempstr = "";
		}
		else if (character != ' ') tempstr += character;
	}
	columnNames += tempstr;
}

void readTableNames(string& tableNames, istringstream& inputCommandStream) {
	string tempstr, tempTableNames = "", str;
	tempstr = "";
	while (str != "|") {
		inputCommandStream >> str;
		if (str != "|") tempTableNames += str + " ";
	}
	tempTableNames.erase(tempTableNames.size() - 1, 1);
	istringstream tableNamesStream(tempTableNames);
	while (tableNamesStream >> tempstr) {
		if (tempstr[tempstr.size() - 1] == ',') tempstr.erase(tempstr.size() - 1, 1);
		tableNames += tempstr + " ";
	}
}

string readCurrentTableName(string tableNames) {
	string currentTableName;
	istringstream stringtableName(tableNames);
	stringtableName >> currentTableName;
	return currentTableName;
}

string cretaeNewTableNames(string tableNames, const string& tableName) {
	tableNames.erase(0, tableName.size() + 1);
	return tableNames;
}

string readCurrentColumnsNames(const string& columnNames, string currentTableName, string& newColumnNames, int32_t amountOfColumn) {
	string currentColumnName, tableName, temp;
	for (unsigned char character : columnNames) {
		if (character == ' ') {
			if (tableName == currentTableName) {
				currentColumnName += temp + ' ';
				amountOfColumn++;
		}
			else newColumnNames += tableName + "." + temp + " ";
			temp = "";
		}
		if (character == '.') {
			tableName = temp;
			temp = "";
		}
		else temp += character;
	}
	if (tableName[0] == ' ') tableName.erase(0, 1);
	if (tableName == currentTableName) {
		currentColumnName += temp + ' ';
		amountOfColumn++;
	}
	else newColumnNames += tableName + "." + temp;
	if (newColumnNames[0] == ' ') newColumnNames.erase(0, 1);
	return currentColumnName;
}

int32_t calculateAmountOfRepeatedRows(const string& tableNames) {
	string tableName;
	int32_t amountOfRepeatedRows = 1;
	if (tableNames == "") return 1;
	istringstream stringTablesaNames(tableNames);
	while (stringTablesaNames >> tableName) {
		int32_t primaryKey = readPrimaryKey(tableName);
		amountOfRepeatedRows *= primaryKey - 1;
	}
	return amountOfRepeatedRows;
}

Hash createHashforFind(const string& tableName, int32_t primaryKey) {
	Hash table;
	int32_t numberFile = primaryKey / 1001 + 1;
	chekTheFileUnlock(tableName);
	lockingTheTable(tableName);
	ifstream tableFile(configuration.name + "/" + tableName + "/" + to_string(numberFile) + ".csv");
	checkTheFileOpening(tableFile);
	string row;
	int32_t currentPK = 1;
	while (getline(tableFile, row)) {
		string columnName;
		istringstream rowStream(row);
		while (getline(rowStream, columnName, ' ')) {
			if (columnName[columnName.size() - 1] == ',') columnName.erase(columnName.size() - 1, 1);
			table.HSET(to_string(currentPK), columnName);
		}
		currentPK++;
	}
	tableFile.close();
	unlockingTheTable(tableName);
	return table;
}

string findElementInTable(const string& tableName, const string& columnName, int32_t primaryKey) {
	Hash table = createHashforFind(tableName, primaryKey);
	int32_t numberColumn = -1;
	for (int32_t i = 0; i < table.HGET("1").size(); i++) {
		if (table.HGET("1").get(i) == columnName) {
			numberColumn = i;
			break;
		}
	}
	if (numberColumn == -1) throw runtime_error("Error column name");
	return table.HGET(to_string(primaryKey + 1)).get(numberColumn);
}

Hash addValueToHash(int32_t findPrimaryKey, int32_t currentPrimaryKey, const string& columnNames, Hash intersectionOfTables, const string& tableName) {
	istringstream columnNamesStream(columnNames);
	string columnName;
	int32_t maxPrimaryKey = readPrimaryKey(tableName) - 1;
	if (findPrimaryKey > maxPrimaryKey) findPrimaryKey = findPrimaryKey - maxPrimaryKey;
	while(columnNamesStream >> columnName) {
		string value = findElementInTable(tableName, columnName, findPrimaryKey);
		intersectionOfTables.HSET(to_string(currentPrimaryKey + 1), value);
	}
	return intersectionOfTables;
}

Hash FindIntersectionOfTables(string tableNames, string columnNames, int32_t& sharedPrimaryKey, Hash intersectionOfTables, bool isFiltering, const string& inputStr) {
	if (tableNames == "") return intersectionOfTables;
	string newColumnsNames;
	int32_t amountOfColumn = 0;
	string currentTableName = readCurrentTableName(tableNames);
	string newTableNames = cretaeNewTableNames(tableNames, currentTableName);
	string currentColumnsNames = readCurrentColumnsNames(columnNames, currentTableName, newColumnsNames, amountOfColumn);
	int32_t AmountOfRepeatedRows = calculateAmountOfRepeatedRows(newTableNames);
	int32_t primaryKey = readPrimaryKey(currentTableName);
	int32_t amountOfRows = 0, currentPrimaryKey = 1, findPrimaryKey = 1;
	while (amountOfRows < sharedPrimaryKey) {
		for (int32_t i = 1; i < primaryKey; i++) {
			for (int32_t j = 0; j < AmountOfRepeatedRows; j++) {
				List<string> logicalExpression = readLogicalExpression(inputStr);
				if (isFiltering and intersectionOfTables.HGET(to_string(currentPrimaryKey)).size() > 1 and intersectionOfTables.HGET(to_string(currentPrimaryKey)).size() < amountOfColumn) intersectionOfTables = addValueToHash(findPrimaryKey, currentPrimaryKey, currentColumnsNames, intersectionOfTables, currentTableName);
				else if (isFiltering and commandWhere(findPrimaryKey + 1, logicalExpression)) intersectionOfTables = addValueToHash(findPrimaryKey, currentPrimaryKey, currentColumnsNames, intersectionOfTables, currentTableName);
				else if (isFiltering == false) intersectionOfTables = addValueToHash(findPrimaryKey, currentPrimaryKey, currentColumnsNames, intersectionOfTables, currentTableName);
				currentPrimaryKey++;
			}
			if (primaryKey != 2) findPrimaryKey++;
		}
		if (primaryKey == 2) amountOfRows += 1;
		else amountOfRows += AmountOfRepeatedRows * (primaryKey - 1);
	}
	if (amountOfRows > sharedPrimaryKey) sharedPrimaryKey = amountOfRows;
	intersectionOfTables = FindIntersectionOfTables(newTableNames, newColumnsNames, sharedPrimaryKey, intersectionOfTables, isFiltering, inputStr);
	return intersectionOfTables;
}

bool readInputDataForFiltering(istringstream& inputCommandStream) {
	if (!chekingCommandWhere(inputCommandStream)) return false;
	return true;
}



void commandSelectFrom(istringstream& inputCommandStream, int32_t clientSocket) {
	string tableNames, columnNames;
	Hash intersectionOfTables;
	readColumnName(columnNames, inputCommandStream);
	readTableNames(tableNames, inputCommandStream);
	int32_t sharedPrimaryKey = 1;
	bool isFiltering = readInputDataForFiltering(inputCommandStream);
	string inputStr;
	getline(inputCommandStream, inputStr);
	intersectionOfTables = FindIntersectionOfTables(tableNames, columnNames, sharedPrimaryKey, intersectionOfTables, isFiltering, inputStr);
	string result = "";
	intersectionOfTables.print(result);
	send(clientSocket, result.c_str(), result.length(), 0);
}

void handlingCommands(string& inputCommand, int32_t clientSocket) {
	istringstream inputCommandStream(inputCommand);
	string command;
	inputCommandStream >> command;
	if (command == "SELECT") {
        commandSelectFrom(inputCommandStream, clientSocket);
    }
	else if (command == "INSERT") {
        commandInsertInto(inputCommandStream);
    }
	else if (command == "DELETE") {
        commandDeleteFrom(inputCommandStream);
    }
	else if (command == "EXIT") return;
	else throw runtime_error("Incorrect command");
}

void processingClientsRequest(int32_t clientSocket, mutex& mtx) {
	while (true) {
		if (!mtx.try_lock()) { 
    		string message = "Another user is working with a table, please wait";
    		send(clientSocket, message.c_str(), message.length(), 0);
			sleep(10);
			continue;
    	}
		char buffer[1024] = {0};
		int received = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
		string message = "Command complete";
    	send(clientSocket, message.c_str(), message.length(), 0);
		buffer[received] = '\0';
		string inputCommand(buffer);
		handlingCommands(inputCommand, clientSocket);
		mtx.unlock();
		if (inputCommand == "EXIT") break;
	}
	cout << "A new client has disconnected: " << clientSocket << endl;
	close(clientSocket);
}

int main() {
    try {
	    setlocale(LC_ALL, "RUSSIAN");
        createDataBase();

        int32_t serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == 0) throw runtime_error("Error create socket");

        sockaddr_in serverAddress;
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(7432);
        serverAddress.sin_addr.s_addr = INADDR_ANY;

        if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) throw runtime_error("Address binding error");
        if (listen(serverSocket, 3) < 0) throw runtime_error("Listening error");

        cout << "The server is listening" << endl;
		mutex mtx;
        while (true) {
            int32_t clientSocket = accept(serverSocket, nullptr, nullptr);
            if (clientSocket < 0) throw runtime_error("failed to accept the data");

            cout << "A new client has connected: " << clientSocket << endl;
            thread t(processingClientsRequest, clientSocket, ref(mtx));
			t.detach();
        }
		close(serverSocket);
    }
    catch (exception &e) {
		    cout << e.what();
	    }
}

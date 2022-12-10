#pragma once
#include "common.h"

class FibSorter {
private:
	DataBase* db;
	ProgressBar* pg;
	int maxNbOfRecordsOnTape;
	int recReadCounter, recWriteCounter;
	int blockReadCounter, blockWriteCounter;
	int phaseCounter;
	int nbOfRecords;
	int buffSize;
	int nbOfRunsOnTapes[NB_OF_TAPES];
	int* nbOfRecordsInRunsOnTapes[NB_OF_TAPES]; // -1 means there is no next run, 0 means a dummy run
	int positionOnTapes[NB_OF_TAPES];
	char** readBuff[NB_OF_TAPES + 1]; // char[NB_OF_TAPES + 1][buffSize][RECORD_LENGTH + 1]
	char** writeBuff[NB_OF_TAPES + 1]; // char[NB_OF_TAPES + 1][buffSize][RECORD_LENGTH + 1]
	int prevReadKey[NB_OF_TAPES + 1], prevWriteKey[NB_OF_TAPES + 1];
	bool useBlockOps;
public:
	FibSorter(int nbOfRecords, short fillMode, short printMode, bool useBlockOps, int buffSize, char* name);

	// frees the allocated memory
	void freeMem(const char* msg);

	// mode takes values: ABOVE_EQUAL, BELOW
	int getFibNumber(int mode, int number);

	// returns 0 when a=b, 1 when a>b and -1 when a<b
	// one string is greater than the other if its character of highest value that doesn't occur in the other string is greater than it's counterpart
	// eg.: aaabed > caaabd, because 'e' is unique and is bigger than 'c' which also is unique
	int compare(char* a, char* b);

	// writes buffSize records from source ( DB, TAPE1, TAPE2, TAPE3 ) to dest
	// when there are no records to read, it writes SPECIAL_CHARs to dest
	// increments the readCounter if DataBase::blockRead() is called
	void blockRead(char* dest, int source, int key);

	// writes buffSize records  to a buffer and from the buffer to dest ( DB, TAPE1, TAPE2, TAPE3 ) if needed
	// increments the writeCounter if DataBase::blockWrite() is called
	void blockWrite(int dest, char* source, int key);

	// makes sure blockWrite() writes records from the specified buffer to dest
	// default value for dest is: DB, it takes values ( DB, TAPE1, TAPE2, TAPE3 )
	void flushBlockWrite(int dest = 0);

	// writes from source ( -1 - db.txt, [0,1,2] - tape[1,2,3].txt ) to dest, decrements the nbOfRecordsInRunsOnTapes[source][run] and increments the key
	// uses the blockRead() method if useBlockOps = true
	void read(char* dest, int source, int& key, int run);

	// writes from source to dest ( -1 - db.txt, [0,1,2] - tape[1,2,3].txt ) and increments the key and the nbOfRecordsInRunsOnTapes[dest][run]
	// uses the blockWrite() method if useBlockOps = true
	void write(int dest, char* source, int& key, int run);

	// mode takes values: START_FINISH, EACH_STEP, SILENT
	// results of the process are going to be printed according to the mode
	void sort(short mode);

	// whichOne: DB, TAPE1, TAPE2, TAPE3
	void printFile(int whichOne);
};
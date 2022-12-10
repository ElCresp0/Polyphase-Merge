#pragma once

class DataBase
{
private:
    char name[MAX_FILE_NAME_LENGTH + 1];
    char tape1[MAX_FILE_NAME_LENGTH + 1];
    char tape2[MAX_FILE_NAME_LENGTH + 1];
    char tape3[MAX_FILE_NAME_LENGTH + 1];
    int size;
    int buffSize;
    bool allGoodVar = true;
public:
    // mode takes values: FILL_FROM_FILE, RANDOM_FILL, FILL_BY_FINGER
    // size is the number of records
    // buff size is the max number of records for disc operations
    DataBase(char* name, short mode, int size, int buffSize);

    // fills the file with randomly generated records of various lengths
    void randomFill();

    // fills the file with user input
    void fingerFill();

    // uses a provided file and controlls its initial correctness
    void fromFileFill();

    // copies all characters from from to to and appends SPECIAL_CHARs on the beginning so that the string has RECORD_SIZE of length
    void myStrCpy(char* from, char* to, int length);

    // deletes SPECIAL_CHARS form the beginning of the str
    void clearSpecialChars(char* str, int length);

    // whichOne takes values: DB, TAPE1, TAPE2, TAPE3, mode is an argument of the fopen_s() function
    FILE* openFile(int whichOne, char mode[3]);
    
    // fills up the tape with empty records and closes the file
    void initializeTape(FILE* tape);
    
    // initializes the tapes (makes sure the files are accessible, calls initializeTape() for each
    void initializeTapes();
    
    // writes buffSize records from source to dest, when there are no records to read, it writes invalid records (rec[0] = SPECIAL_CHAR) to dest
    // source takes values: DB, TAPE1, TAPE2, TAPE3
    void blockRead(char* dest, int source, int key);

    // writes buffSize records from dest to source, doesn't write invalid records (rec[0] = SPECIAL_CHAR)
    // dest takes values: DB, TAPE1, TAPE2, TAPE3
    void blockWrite(int dest, char* source, int key);

    // source takes values: DB, TAPE1, TAPE2, TAPE3
    // a record without the padding is written to dest
    void read(char* dest, int source, int key);

    // dest takes values: DB, TAPE1, TAPE2, TAPE3
    // a record with a padding is written to dest
    void write(int dest, char* source, int key);

    // returns the allGoodVar field which is set to false when something goes wrong
    bool allGood();
};


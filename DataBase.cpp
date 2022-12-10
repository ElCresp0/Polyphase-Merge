#include "common.h"

DataBase::DataBase(char* name, short mode, int size, int buffSize) {
    this->size = size;
    this->buffSize = buffSize;
    strcpy_s(this->name, name);
    strcpy_s(this->tape1, "tape1.txt");
    strcpy_s(this->tape2, "tape2.txt");
    strcpy_s(this->tape3, "tape3.txt");

    initializeTapes();

    switch (mode) {
    case FILL_FROM_FILE:
        fromFileFill();
        break;
    case RANDOM_FILL:
        randomFill();
        break;
    case FILL_BY_FINGER:
        fingerFill();
        break;
    default:
        randomFill();
        break;
    }
}

void DataBase::randomFill() {
    srand(time(NULL));
    for (int i = 0; i < size; i++) {
        char rec[RECORD_SIZE + 1] = "";
        int randomSize = rand() % (RECORD_SIZE + 1);
        int j;
        for (j = 0; j < randomSize; j++) {
            rec[j] = (rand() % 95) + 32; // ascii characters from range [32;126]
        }
        rec[randomSize] = '\0';
        write(DB, rec, i);
    }
}

void DataBase::fingerFill() {
    for (int i = 0; i < size; i++) {
        char rec[RECORD_SIZE + 2]; // leaves the place for an additional '\n' character because fgets() is used
        printf("type in a char string (max length: %i)\n", RECORD_SIZE);
        fgets(rec, (unsigned)_countof(rec), stdin);
        rec[((std::string)rec).length() - 1] = '\0'; // fgets counts in the \n character from input
        rec[RECORD_SIZE] = '\0';

        int length = ((std::string)rec).length();
        for (int j = 0; j < length; j++) {
            if (rec[j] < 32 || rec[j] > 126) { // ascii characters outside of range [32;126] are illegal
                rec[j] = '#';
            }
        }
        write(DB, rec, i);
    }
}

void DataBase::fromFileFill() {
    short correct = 0;
    int test;
    char key[KEY_SIZE + 1] = "0000";
    char rec[RECORD_SIZE + 1];
    FILE* db;

    fopen_s(&db, name, "r");
    if (db == nullptr) {
        printf("opening file unsuccessful: %s\n", name);
        allGoodVar = false;
        return;
    }

    for (int i = 0; i < size; i++) {
        test = fscanf_s(db, "%4s", key, (unsigned)_countof(key));
        if (atoi(key) != i) correct++;
        if (test == 0) correct++;

        test = fscanf_s(db, "%30c", rec, (unsigned)_countof(rec)); // %30c because ' ' is legal
        rec[RECORD_SIZE] = '\0';
        clearSpecialChars(rec,RECORD_SIZE + 1);
        for (int j = 0; j < std::string(rec).length(); j++) {
            if (rec[j] < 32 || rec[j] > 126) correct++; // ascii characters outside of range [32;126] are illegal
        }

        if (test == 0 || test == -1) correct++;
    }

    if (correct != 0) {
        printf("the file is not correct (%i)\n", correct);
        allGoodVar = false;
    }
    else printf("the file is correct\n");

    fclose(db);
}

void DataBase::myStrCpy(char* from, char* to, int length) {
    int i = 0;
    
    if (from[0] == '\0') {
        myStrCpy((char*)"x", to, length);
        to[length - 2] = SPECIAL_CHAR_FOR_PADDING;
        return;
    }
    while (i < length - ((std::string)from).length() - 1) {
        to[i] = SPECIAL_CHAR_FOR_PADDING;
        i++;
    }
    for (int j = 0; i < length; i++) {
        to[i] = from[j++];
    }
}

void DataBase::clearSpecialChars(char* str, int length) {
    int pad = 0;
    while (str[pad] == (char)SPECIAL_CHAR_FOR_PADDING) pad++;
    for (int i = pad; i < length; i++) {
        str[i - pad] = str[i];
    }
}

FILE* DataBase::openFile(int whichOne, char mode[3]) {
    FILE* db;
    switch (whichOne)
    {
    case DB:
        fopen_s(&db, name, mode);
        break;
    case TAPE1:
        fopen_s(&db, tape1, mode);
        break;
    case TAPE2:
        fopen_s(&db, tape2, mode);
        break;
    case TAPE3:
        fopen_s(&db, tape3, mode);
        break;
    default:
        // whichOne = DB;
        fopen_s(&db, name, mode);
        break;
    }
    return db;
}

void DataBase::initializeTape(FILE* tape) {
    for (int i = 0; i < size; i++) {
        char init[RECORD_SIZE + 1];
        for (int i = 0; i < RECORD_SIZE + 1; i++) {
            init[i] = '#';
        }
        init[RECORD_SIZE] = '\0';
        fprintf_s(tape, "%04i%30s", i, init);
    }
    fclose(tape);
}

void DataBase::initializeTapes() {
    bool fileErr = false;
    FILE* testFile;
    for (int i = TAPE1; i <= TAPE3; i++) {
        testFile = openFile(i, (char*)"w");
        if (testFile == NULL) fileErr = true;
        else initializeTape(testFile);
    }
    
    if (fileErr) {
        printf("errors occured when creating files (tapes)\n");
        allGoodVar = false;
    }
}

void DataBase::blockRead(char* dest, int source, int key) {
    // dest is going to look like: char[30],'\0',char[30],'\0',... buffSize times
    for (int i = 0; i < buffSize; i++) {
        read(dest, source, key + i);
        dest += (RECORD_SIZE + 1);
    }
}

void DataBase::blockWrite(int dest, char* source, int key) {
    // source looks like: char[30],'\0',char[30],'\0',... buffSize times
    for (int i = 0; i < buffSize; i++) {
        if (source[0] != (char)SPECIAL_CHAR) {
            write(dest, source, key + i);
            source += (RECORD_SIZE + 1);
        }
        else break;
    }
}

void DataBase::read(char dest[RECORD_SIZE + 1], int source, int key) {
    int test;
    char rec[RECORD_SIZE + 1] = "_reading error";
    rec[0] = SPECIAL_CHAR;
    fpos_t position = (long long)(RECORD_SIZE + KEY_SIZE) * (long long)key + (long long)KEY_SIZE;
    FILE* db = openFile(source, (char*)"r");
    if (db == nullptr) {
        printf("opening file unsuccessful\n");
        allGoodVar = false;
        return;
    }

    fsetpos(db, &position);

    test = fscanf_s(db, "%30c", rec, (unsigned)_countof(rec));
    myStrCpy(rec, dest, RECORD_SIZE + 1);
    clearSpecialChars(dest, RECORD_SIZE + 1);

    fclose(db);
}

void DataBase::write(int dest, char* source, int key) {
    char rec[RECORD_SIZE + 1];
    fpos_t position = (long long)key * (long long)(RECORD_SIZE + KEY_SIZE);
    FILE* db = openFile(dest, (char*)"r+");
    if (db == nullptr) {
        printf("opening file unsuccessful\n");
        allGoodVar = false;
        return;
    }

    myStrCpy(source, rec, RECORD_SIZE + 1);

    fsetpos(db, &position);
    fprintf_s(db, "%04i%30s", key, rec);
    fclose(db);
}

bool DataBase::allGood() {
    return allGoodVar;
}
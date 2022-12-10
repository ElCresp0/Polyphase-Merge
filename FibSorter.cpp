#include "common.h"

FibSorter::FibSorter(int nbOfRecords, short fillMode, short printMode, bool useBlockOps, int buffSize, char* name) {
    this->useBlockOps = useBlockOps;
    if (buffSize > 0) this->buffSize = buffSize;
    else buffSize = 4;
    blockReadCounter = blockWriteCounter = recReadCounter = recWriteCounter = phaseCounter = 0;
    for (int i = 0; i < NB_OF_TAPES + 1; i++) prevReadKey[i] = prevWriteKey[i] = -2;

    for (int i = 0; i < NB_OF_TAPES; i++) nbOfRunsOnTapes[i] = 0;
    db = (DataBase*)malloc(sizeof(DataBase));
    if (db == NULL) {
        printf("error: malloc (DataBase)\n");
        return;
    }
    pg = (ProgressBar*)malloc(sizeof(ProgressBar));
    if (pg == NULL) {
        freeMem("error: malloc (ProgressBar)\n");
        return;
    }

    if (nbOfRecords < 0 || nbOfRecords > 9999) {
        printf("illegal number of records, it'll be set to: 150\n");
        this->nbOfRecords = 150;
    }
    else this->nbOfRecords = nbOfRecords;
    maxNbOfRecordsOnTape = this->nbOfRecords;

    short mode;

    if (fillMode < 0 || fillMode > 2) {
        printf("illegal input (fill mode), it'll be set to: RANDOM_FILL\n");
        mode = RANDOM_FILL;
    }
    else mode = fillMode;

    for (int i = 0; i < NB_OF_TAPES; i++) {
        nbOfRecordsInRunsOnTapes[i] = (int*)malloc((long long)maxNbOfRecordsOnTape * sizeof(int));
        if (nbOfRecordsInRunsOnTapes[i] == NULL) {
            freeMem("error: malloc (nbOfRecordsInRunsOnTapes)\n");
            return;
        }
        for (int j = 0; j < maxNbOfRecordsOnTape; j++) {
            nbOfRecordsInRunsOnTapes[i][j] = -1;
        }
    }
    for (int i = 0; i < NB_OF_TAPES + 1; i++) {
        readBuff[i] = (char**)malloc((long long)buffSize * sizeof(char*));
        if (readBuff[i] == NULL) {
            freeMem("error: malloc");
            return;
        }
        writeBuff[i] = (char**)malloc((long long)buffSize * sizeof(char*));
        if (writeBuff[i] == NULL) {
            freeMem("error: malloc");
            return;
        }
    }
    
    for (int i = 0; i < NB_OF_TAPES + 1; i++) {
        for (int j = 0; j < buffSize; j++) {
            writeBuff[i][j] = (char*)malloc((RECORD_SIZE + 1) * sizeof(char));
            if (writeBuff[i][j] == NULL) {
                freeMem("error: malloc");
                return;
            }
            readBuff[i][j] = (char*)malloc((RECORD_SIZE + 1) * sizeof(char));
            if (readBuff[i][j] == NULL) {
                freeMem("error: malloc");
                return;
            }
        }
    }
    
    *db = DataBase(name, mode, this->nbOfRecords, this->buffSize);
    if (db->allGood() == false) {
        freeMem("error: failed to initialize the database\n");
        return;
    }
    if (printMode < 0 || printMode > 2) {
        printf("illegal input (printing mode), the mode will be set to: START_FINISH\n");
        mode = 0;
    }
    else mode = printMode;

    sort(mode);
    freeMem("");
}

void FibSorter::freeMem(const char* msg) {
    printf(msg);
    for (int i = 0; i < NB_OF_TAPES + 1; i++) {
        for (int j = 0; j < buffSize; j++) {
            if (readBuff != NULL && readBuff[i] != NULL && readBuff[i][j] != NULL) free(readBuff[i][j]);
            if (writeBuff != NULL && writeBuff[i] != NULL && writeBuff[i][j] != NULL) free(writeBuff[i][j]);
        }
        if (readBuff != NULL && readBuff[i] != NULL) free(readBuff[i]);
        if (writeBuff != NULL && writeBuff[i] != NULL) free(writeBuff[i]);
    }

    for (int i = 0; i < NB_OF_TAPES; i++) {
        if (nbOfRecordsInRunsOnTapes[i] != NULL) free(nbOfRecordsInRunsOnTapes[i]);
    }
    if (db != NULL) free(db);
    if (pg != NULL) free(pg);
}

int FibSorter::getFibNumber(int mode, int number) {
    int a = 1;
    int b = 1;
    int mod = 0;
    while (a < number && b < number) {
        mod++;
        if ((mod % 2) == 0) a += b;
        else b += a;
    }
    if ( mode == ABOVE_EQUAL )
        return (a > b) ? a : b;
    else return (a < b) ? a : b;
}

int FibSorter::compare(char* a, char* b) {
    char uniqueInA[RECORD_SIZE + 1] = "";
    char uniqueInB[RECORD_SIZE + 1] = "";
    int uniqueIters[2] = {0, 0};
    char* arrs[2] = { a, b };
    char* uniqueArrs[2] = { uniqueInA, uniqueInB };
    char bestOfs[3] = "";
    for (int arr = 0; arr < 2; arr++) {
        for (int i = 0; i < RECORD_SIZE; i++) {
            char c = arrs[arr][i];
            bool unique = true;
            if (((std::string)arrs[(arr + 1) % 2]).find(c) != std::string::npos)
                unique = false;
            if (unique && ((std::string)uniqueArrs[arr]).find(c) == std::string::npos)
                uniqueArrs[arr][uniqueIters[arr]++] = c;
        }
    }
    for (int i = 0; i < 2; i++) {
        uniqueArrs[i][uniqueIters[i]] = '\0';
    }
    for (int i = 0; i < 2; i++) {
        bestOfs[i] = uniqueArrs[i][0];
        for (int j = 1; j < uniqueIters[i]; j++) {
            if (uniqueArrs[i][j] > bestOfs[i]) bestOfs[i] = uniqueArrs[i][j];
        }
    }
    if (bestOfs[0] > bestOfs[1]) return 1;
    else if (bestOfs[0] < bestOfs[1]) return -1;
    else return 0;
}

void FibSorter::blockRead(char* dest, int source, int key) {
    if (prevWriteKey[source] != -2 // checking for presence of the record in the writing buffer
        && key >= prevWriteKey[source]
        && key - prevWriteKey[source] < buffSize
        && writeBuff[source][key - prevWriteKey[source]][0] != (char)SPECIAL_CHAR) { // SPECIAL_CHAR means the slot is empty
        sprintf_s(dest, RECORD_SIZE + 1, "%s", writeBuff[source][key - prevWriteKey[source]]);
        return;
    }
    
    int keyInTheBuffer;
    if (prevReadKey[source] >= 0 // checking for presence of the record in the reading buffer
        && key >= prevReadKey[source]
        && key - prevReadKey[source] < buffSize) {
        if (readBuff[source][key - prevReadKey[source]][0] == (char)SPECIAL_CHAR) {
            sprintf_s(dest, RECORD_SIZE + 1, "%s", ">>>>>>INVALID RECORD<<<<<<");
            return;
        }
        keyInTheBuffer = key - prevReadKey[source]; // the record is already available in the buffer
    }
    else {
        // buffSize records are going to be written to the buffer
        char* tmp = (char*)malloc(buffSize * (RECORD_SIZE + 1) * sizeof(char));
        if (tmp == NULL) {
            printf("malloc error\n");
            sprintf_s(dest, RECORD_SIZE + 1, "%s", ">>>>>>INVALID RECORD<<<<<<");
            return;
        }
        char* tmpPtr = tmp;
        db->blockRead(tmp, source, key);
        for (int i = 0; i < buffSize; i++) {
            sprintf_s(readBuff[source][i], RECORD_SIZE + 1, "%s", tmpPtr);
            tmpPtr += (RECORD_SIZE + 1);
        }
        blockReadCounter++;
        keyInTheBuffer = 0;
        prevReadKey[source] = key;
        free(tmp);
    }
    sprintf_s(dest, RECORD_SIZE + 1, "%s", readBuff[source][keyInTheBuffer]);
}

void FibSorter::blockWrite(int dest, char* source, int key) {
    if ((key >= prevWriteKey[dest] && key - prevWriteKey[dest] < buffSize)
        || prevWriteKey[dest] < 0) {
        if (prevWriteKey[dest] < 0) {
            // initializing the buffer with invalid records
            prevWriteKey[dest] = key;
            for (int i = 0; i < buffSize; i++) {
                sprintf_s(writeBuff[dest][i], RECORD_SIZE + 1, "##############################");
                writeBuff[dest][i][0] = SPECIAL_CHAR;
            }
        }
        sprintf_s(writeBuff[dest][key - prevWriteKey[dest]], RECORD_SIZE + 1, "%s", source);
    }
    else { // there is no space for the new record in the buffer
        flushBlockWrite(dest);
        blockWrite(dest, source, key);
    }
}

void FibSorter::flushBlockWrite(int dest) {
    if (prevWriteKey[dest] < 0 || dest < 0 || dest > 3) return;
    
    char* tmp = (char*)malloc((long long)buffSize * (RECORD_SIZE + 1) * sizeof(char));
    if (tmp == NULL) {
        printf("error: malloc (FibSorter.cpp line 225)\n");
        for (int i = 0; i < buffSize; i++) {
            db->write(dest, writeBuff[dest][i], prevWriteKey[dest] + i);
        }
        blockWriteCounter++;
        return;
    }
    char* tmpPtr = tmp;
    int i = 0;
    for (; i < buffSize; i++) {
        sprintf_s(tmpPtr, RECORD_SIZE + 1, "%s", writeBuff[dest][i]);
        if (tmpPtr[0] == (char)SPECIAL_CHAR) tmpPtr[0] = '#';
        tmpPtr += (RECORD_SIZE + 1);
    }

    // updating the reading buffer if needed
    if (prevWriteKey[dest] >= 0
        && abs(prevWriteKey[dest] - prevReadKey[dest]) < buffSize) {
        for (int j = prevWriteKey[dest]; j < prevWriteKey[dest] + buffSize; j++) {
            for (int k = prevReadKey[dest]; k < prevReadKey[dest] + buffSize; k++) {
                if (j == k) {
                    sprintf_s(readBuff[dest][k - prevReadKey[dest]], RECORD_SIZE + 1, "%s", writeBuff[dest][j - prevWriteKey[dest]]);
                }
            }
        }
    }

    db->blockWrite(dest, tmp, prevWriteKey[dest]);
    blockWriteCounter++;

    prevWriteKey[dest] = -1; // signaling the buffer is empty now
    free(tmp);
}

void FibSorter::read(char* dest, int source, int& key, int run) {
    if (useBlockOps == true)
        blockRead(dest, source + 1, key);
    else {
        db->read(dest, source + 1, key);
    }

    recReadCounter++;
    key = (key + 1);
    if (source != -1) nbOfRecordsInRunsOnTapes[source][run] -= (nbOfRecordsInRunsOnTapes[source][run] == 1) ? 2 : 1; // -1 for no runs and 0 for dummy runs
}

void FibSorter::write(int dest, char* source, int& key, int run) {
    if (useBlockOps == true)
        blockWrite(dest + 1, source, key);
    //flushBlockWrite();
    else {
        db->write(dest + 1, source, key);
    }
    

    recWriteCounter++;
    key = (key + 1);
    if (dest != -1) nbOfRecordsInRunsOnTapes[dest][run] += (nbOfRecordsInRunsOnTapes[dest][run] == -1) ? 2 : 1; // -1 for no runs and 0 for dummy runs
}

void FibSorter::sort(short mode) {
    int dummyRuns = 0;
    int mainTape, secondaryTape, destTape;
    char rec1[RECORD_SIZE + 1] = "";
    char rec2[RECORD_SIZE + 1] = "";
    char* recPtrPrev = rec1, * recPtrNext = rec2;
    int iter = 0;
    int iters[2] = { 0, 0 };

    //distribution of the runs
    // > put the runs evenly on 2 tapes
    // > calculate the desired numbers of runs on tapes (using sum of runs and fib)
    // > put some of the runs from one tape to another and fill one of the tapes with dummy runs if needed

    mainTape = 0;
    read(recPtrPrev, -1, iter, NULL); // iter++
    write(mainTape, recPtrPrev, iters[mainTape], nbOfRunsOnTapes[mainTape]);
    for (; iters[0] + iters[1] < nbOfRecords;) {
        read(recPtrNext, -1, iter, NULL); // iter++
        int cmp = compare(recPtrPrev, recPtrNext);
        if (cmp > 0) {
            nbOfRunsOnTapes[mainTape]++;
            if (nbOfRunsOnTapes[mainTape] == getFibNumber(ABOVE_EQUAL, nbOfRunsOnTapes[mainTape])) mainTape = (mainTape + 1) % 2;
        }
        write(mainTape, recPtrNext, iters[mainTape], nbOfRunsOnTapes[mainTape]); // iters[main]++
        std::swap(recPtrPrev, recPtrNext);
    }
    nbOfRunsOnTapes[mainTape]++;

    int desiredNbOfRunsOnTapes[2];
    int initialNbOfRuns;
    desiredNbOfRunsOnTapes[0] = getFibNumber(BELOW, getFibNumber(ABOVE_EQUAL, nbOfRunsOnTapes[0] + nbOfRunsOnTapes[1]));
    desiredNbOfRunsOnTapes[1] = getFibNumber(BELOW, desiredNbOfRunsOnTapes[0]);
    initialNbOfRuns = desiredNbOfRunsOnTapes[0] + desiredNbOfRunsOnTapes[1];
    *pg = ProgressBar((int)(1.45 * log2((double)initialNbOfRuns)));

    mainTape = 0; secondaryTape = 1;
    if (nbOfRunsOnTapes[1] < desiredNbOfRunsOnTapes[1]) std::swap(mainTape, secondaryTape);
    while (nbOfRunsOnTapes[1] != desiredNbOfRunsOnTapes[1]) {
        int goBack = nbOfRecordsInRunsOnTapes[secondaryTape][nbOfRunsOnTapes[secondaryTape] - 1];
        iters[secondaryTape] -= goBack;
        while (nbOfRecordsInRunsOnTapes[secondaryTape][nbOfRunsOnTapes[secondaryTape] - 1] > 0) {
            read(rec1, secondaryTape, iters[secondaryTape], nbOfRunsOnTapes[secondaryTape] - 1); // iters[sec]++
            write(mainTape, rec1, iters[mainTape], nbOfRunsOnTapes[mainTape]); // iters[main]++
        }
        iters[secondaryTape] -= goBack;
        nbOfRunsOnTapes[secondaryTape]--;
        nbOfRunsOnTapes[mainTape]++;
    }
    mainTape = 0; secondaryTape = 1;

    // the dummy runs are not actually written to the disc
    while (nbOfRunsOnTapes[mainTape] < desiredNbOfRunsOnTapes[mainTape]) {
        nbOfRecordsInRunsOnTapes[mainTape][nbOfRunsOnTapes[mainTape]] = 0;
        nbOfRunsOnTapes[mainTape]++;
        dummyRuns++;
    }

    printf("initial number of runs on each tape: %i(%i), %i, %i\n", nbOfRunsOnTapes[mainTape], dummyRuns, nbOfRunsOnTapes[secondaryTape], nbOfRunsOnTapes[2]);
    
    if (mode != SILENT) {
        printf("the file before sorting:\n\n");
        printFile(DB);
    }
    if (mode == EACH_STEP) {
        printf("tape1:\n");
        printFile(TAPE1);
        printf("tape2:\n");
        printFile(TAPE2);
        printf("tape3:\n");
        printFile(TAPE3);
    }

    for (int i = 0; i < NB_OF_TAPES; i++) positionOnTapes[i] = 0;
    mainTape = 1;
    secondaryTape = 2;
    destTape = 0;

    // one iteration is one phase of the sort
    while (true) {
        int currentRunInTape[NB_OF_TAPES] = { 0, 0, 0 };
        bool freeMain = true;
        bool freeSec = true;

        // main tape is the one with the biggest amount of runs
        // secondary tape is the one with the second biggest amount
        // dest tape is the one with no runs
        // the tapes switch these roles cyclicly
        mainTape = (mainTape + 2) % 3;
        secondaryTape = (secondaryTape + 2) % 3;
        destTape = (destTape + 2) % 3;
        
        // merging nbOfRunsOnTapes[secondaryTape] runs
        // from main and secondary tapes into the destTape
        while (nbOfRunsOnTapes[secondaryTape] > 0) {

            // merging current 2 runs
            while (true) {
                // setting rec1 and rec2 strings
                if (freeMain && nbOfRecordsInRunsOnTapes[mainTape][currentRunInTape[mainTape]] > 0) {
                    read(rec1, mainTape, positionOnTapes[mainTape], currentRunInTape[mainTape]);
                    freeMain = false;
                }
                else if (freeMain) rec1[1] = SPECIAL_CHAR;
                if (freeSec && nbOfRecordsInRunsOnTapes[secondaryTape][currentRunInTape[secondaryTape]] > 0) {
                    read(rec2, secondaryTape, positionOnTapes[secondaryTape], currentRunInTape[secondaryTape]);
                    freeSec = false;
                }
                else if (freeSec) rec2[1] = SPECIAL_CHAR;

                // writing to destTape
                if (freeMain && freeSec && rec1[1] == (char)SPECIAL_CHAR && rec2[1] == (char)SPECIAL_CHAR) {
                    break;
                }
                else if (freeSec == false && rec1[1] == (char)SPECIAL_CHAR) { // which means there are no more records in the current run on the main tape
                    write(destTape, rec2, positionOnTapes[destTape], currentRunInTape[destTape]);
                    freeSec = true;
                }
                else if (freeMain == false && rec2[1] == (char)SPECIAL_CHAR) { // which means there are no more records in the current run on the 2nd tape
                    write(destTape, rec1, positionOnTapes[destTape], currentRunInTape[destTape]);
                    freeMain = true;
                }
                else if (freeMain == false && freeSec == false) {
                    int from = (compare(rec1, rec2) < 0) ? mainTape : secondaryTape;
                    write(destTape, (from == mainTape) ? rec1 : rec2, positionOnTapes[destTape], currentRunInTape[destTape]);
                    if (from == mainTape) freeMain = true;
                    else freeSec = true;
                }            
            }
        
            freeMain = freeSec = true;
            currentRunInTape[mainTape]++;
            currentRunInTape[secondaryTape]++;
            currentRunInTape[destTape]++;
            nbOfRunsOnTapes[mainTape]--;
            nbOfRunsOnTapes[secondaryTape]--;
            nbOfRunsOnTapes[destTape]++;
        }
        phaseCounter++;
        positionOnTapes[destTape] = positionOnTapes[secondaryTape] = 0;
        // position in mainTape remains the same as some records are left there

        // shifting the nbOfRecordsInRunsOnTapes[mainTape] array so that the first remaining run of the tape is the first one of the array
        iter = 0;
        while (nbOfRecordsInRunsOnTapes[mainTape][iter] < 0 && iter < maxNbOfRecordsOnTape) iter++;
        for (int i = iter; i < maxNbOfRecordsOnTape; i++)
            nbOfRecordsInRunsOnTapes[mainTape][i - iter] = nbOfRecordsInRunsOnTapes[mainTape][i];
        for (int i = maxNbOfRecordsOnTape - iter; i < maxNbOfRecordsOnTape; i++)
            nbOfRecordsInRunsOnTapes[mainTape][i] = -1;
 
        if (mode == EACH_STEP) {
            printf("after %i phases:\nnumber of runs on each tape: %i, %i, %i\n", phaseCounter, nbOfRunsOnTapes[0], nbOfRunsOnTapes[1], nbOfRunsOnTapes[2]);
            printf("tape1:\n");
            printFile(TAPE1);
            printf("tape2:\n");
            printFile(TAPE2);
            printf("tape3:\n");
            printFile(TAPE3);
            // waits for user to press enter after each phase
            // fgets(buffForFgets, 2, stdin);
        }
        pg->nextStep(stdout); // printing the progress bar
        
        // check if the sorting has been completed
        if (nbOfRunsOnTapes[destTape] == 1 && nbOfRunsOnTapes[mainTape] == 0) {
            break;
        }
    }

    // saving the records back to the original file
    for (int i = 0; i < nbOfRecords;) {
        read(rec1, destTape, positionOnTapes[destTape], 0);
        write(-1, rec1, i, 0); // i++
    }

    if (useBlockOps) {
        for (int i = 0; i < NB_OF_TAPES + 1; i++ ) flushBlockWrite(i);
    }

    if (mode != SILENT) {
        printf("the file after sorting:\n");
        printFile(DB);
        printf("\n\n");
    }

    // these operations of reading are not going to be counted as the checking is not a part of the algorithm 
    printf("checking correctness of the sort...\n");
    char* recPtr1 = rec1;
    char* recPtr2 = rec2;
    int correct = 0;                                        // the higher the value
    db->read(rec1, DB, 0);                                  // the less the correctness
    db->read(rec2, DB, 1);
    for (int i = 2; i < nbOfRecords; i++) {
        correct += (compare(recPtr1, recPtr2) > 0) ? 1 : 0; // as it increments each time it finds
        std::swap(recPtr1, recPtr2);                        // adjacent records that are out of order
        db->read(recPtr2, DB, i);
    }

    printf("the records are%s sorted correctly (%i)\n\n",
        (correct == 0) ? "" : " not", correct);
    printf("statistics:\n%i records, %i(%i) initial runs, %i phases\n",
        nbOfRecords, initialNbOfRuns, dummyRuns, phaseCounter);
    printf("%i record reads, %i record writes, %i block reads, %i block writes\n",
        recReadCounter, recWriteCounter, blockReadCounter, blockWriteCounter);
    printf("resulting in total of %i operations on records and %i disc operations\n",
        recReadCounter + recWriteCounter, blockReadCounter + blockWriteCounter);
    // 1.45 log2 r
    printf("expected number of phases was: %f\n",
        1.45 * log2((double)initialNbOfRuns));
    // 2N (1,04 log2 r + 1) / b; N - nb of records, r - initial nb of runs (same as N), b = B / R
    printf("expected number of disc operations was: %f\n",
        2.0 * (double)nbOfRecords * (1.04 * log2((double)initialNbOfRuns) + 1.0) / (double)buffSize);
}

void FibSorter::printFile(int whichOne) {
    int pos = 0;
    for (int i = 0; i < maxNbOfRecordsOnTape; i++) {
        char rec[RECORD_SIZE + 1];
        if (prevWriteKey[whichOne] >= 0 && prevWriteKey[whichOne] <= i && i - prevWriteKey[whichOne] < buffSize) {
            sprintf_s(rec, RECORD_SIZE + 1, "%s", writeBuff[whichOne][i - prevWriteKey[whichOne]]);
            if (rec[0] == SPECIAL_CHAR) rec[0] = '#';
        }
        else if (prevReadKey[whichOne] >= 0 && prevReadKey[whichOne] <= i && i - prevReadKey[whichOne] < buffSize) {
            sprintf_s(rec, RECORD_SIZE + 1, "%s", readBuff[whichOne][i - prevReadKey[whichOne]]);
            if (rec[0] == SPECIAL_CHAR) rec[0] = '#';
        }
        else db->read(rec, whichOne, i);
        printf("%30s\n", rec);
    }
}
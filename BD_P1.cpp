// Struktury baz danych
// Projekt 1 - sortowanie polifazowe
// Jakub Kilianczyk, 184301

#include "common.h"

// main function takes arguments:
// - number of records (max 9999)
// - fill mode: 0-random, 1-from file, 2-manually
// - printing mode: 0-start_finish, 1-each_step, 2-silent
// - use of block disk operations: 0/1
// - max number of records in one block operation
// - name of the file to use (relevant if fill mode == from file)
int main(int argc, char* argv[])
{
    if (argc > 1 && std::string(argv[1]).compare("-h") == 0) {
        printf("main function takes arguments:\n- number of records (max 9999)\n- fill mode: 0-random, 1-from file, 2-manually\n- printing mode: 0-start_finish, 1-each_step, 2-silent\n- use of block disk operations: 0/1\n- max number of records in one block operation\n- name of the file to use (relevant if fill mode == from file)");
        return 0;
    }

    int nbOfRecords = (argc > 1) ? atoi(argv[1]) : 150;
    int fill = (argc > 2) ? atoi(argv[2]) : RANDOM_FILL;
    int printing = (argc > 3) ? atoi(argv[3]) : START_FINISH;
    int useBlockOps = (argc > 4) ? atoi(argv[4]) : USE_BLOCK_OPS;
    int buffSize = (argc > 5) ? atoi(argv[5]) : 4;
    char name[MAX_FILE_NAME_LENGTH + 1];
    if (argc > 6) sprintf_s(name, MAX_FILE_NAME_LENGTH + 1, argv[6]);
    else sprintf_s(name, MAX_FILE_NAME_LENGTH + 1, "bd.txt");

    FibSorter f = FibSorter(nbOfRecords, (short)fill, (short)printing, (useBlockOps == USE_BLOCK_OPS) ? true : false, buffSize, name);

    return 0;
}

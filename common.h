#pragma once

#define RECORD_SIZE 30 // watch out for printf("%30s") when increasing the value
#define KEY_SIZE 4
#define NB_OF_TAPES 3
#define SPECIAL_CHAR 205 // a special char to mark invalid records
#define SPECIAL_CHAR_FOR_PADDING 204 // a special character that is used as a padding for shorter records
#define MAX_FILE_NAME_LENGTH 15

enum fill_mode {
	RANDOM_FILL,
	FILL_FROM_FILE,
	FILL_BY_FINGER
};
enum fib_mode {
	ABOVE_EQUAL,
	BELOW
};
enum print_mode {
	START_FINISH,
	EACH_STEP,
	SILENT
};
enum file {
	DB,
	TAPE1,
	TAPE2,
	TAPE3
};
enum block_ops {
	DONT_USE_BLOCK_OPS,
	USE_BLOCK_OPS
};

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <time.h>
#include "DataBase.h"
#include "ProgressBar.h"
#include "FibSorter.h"
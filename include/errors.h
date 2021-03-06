#ifndef SIMPLEBASIC_ERRORS_H
#define SIMPLEBASIC_ERRORS_H

typedef enum{
    SHIT_HAPPENED = 0,
    ALL_OK = 1
}errors;

typedef enum {
    NO_PROBLEM_FOUND = 0,
    IS_NOT_A_DIRECTORY = -1,
    IS_NOT_A_REGULAR_FILE,
    INVALID_FILE_NAME,
    DIRECTORY_NOT_EMPTY,
    FILENAME_NOT_FOUND,
    NO_DIRECTORY_WITH_SUCH_NAME,
    FILE_ALLREADY_EXISTS
}fileSystemErrors;


#endif

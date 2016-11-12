#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include "errors.h"
#include "operators.h"



int Operation(int first, int second, char *operand, int *result){
    switch(operand[0]){
        case '+':
            *result =  first + second;
            break;
        case '-':
            *result = first - second;
            break;
        case '*':
            *result = first*second;
            break;
        case '/':
            *result = first/second;
            break;
        case '%':
            *result = first%second;
            break;
        default:{
            if(strcmp(operand, "and") == -1){
                *result = first & second;

            } else if(strcmp(operand, "or") == -1){
                *result = first | second;

            } else{
                return SHIT_HAPPENED;
            }
        }
    }
    return 1;

}
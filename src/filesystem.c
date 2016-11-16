#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include "filesystem.h"
#include "errors.h"

int maximumInode = 0;

/*
 * 1 - вспомогательные функции
 */


/*
 * @DESCRIPTION: на вход подаётся разделитель, исходная строка и буфер
 * Копирует в буфер нуль-терминированный кусок строки - часть исходной до разделителя.
 */
char *parse_string(const char delim, char *src, char *dest){
    while(*(src) != delim && *src != 0)
        *(dest++) = *(src++);
    *dest = '\0';
    return src;
}//do we need two realisations?


/*
 * 2 - добавление новых файлов в ФС.
 */

file* init_file_system(){
    file *home;
    new_file(NULL, "/", 'd', &home);
    return home;
}

void add_simple_record(record *previous, file *child){
    record *temp = malloc(sizeof(record));
    temp->previous = previous;
    temp->current = child;
    temp->next = NULL;
    previous->next = temp;
    ++previous->current->fileSize;
}//approved

void add_catatlog_record(record *parent_directory_record, file *new_directory){
    record *new_directory_record = malloc(sizeof(record));
    new_directory_record->previous = parent_directory_record ? parent_directory_record : new_directory_record;
    new_directory_record->current = new_directory;
    new_directory->content = malloc(sizeof(void*));
    *(record**)(new_directory->content) = new_directory_record;
    new_directory_record->next = NULL;
    ++new_directory_record->current->fileSize;
}//approved

/*
 * @DESCRIPTION: на вход подаётся запись - '.' - информация о текущем каталоге
 * имя файла, которое мы хотим создать, тип файла и длину имени файла.
 */


char new_file(file *current_directory, char *filename, char type, file **created_file){
    record *record_to_find;
    char error_code;
    if(current_directory != NULL){
        error_code = find_record(&filename, current_directory, &record_to_find);
        //printf("File_Name: %s\n",filename);
        if(error_code != FILENAME_NOT_FOUND){
            return error_code;
        }
    } else{

        record_to_find = NULL;
    }
    *created_file = malloc(sizeof(file));
    (*created_file)->inode = maximumInode;
    ++maximumInode;
    size_t name_len = strlen(filename);
    (*created_file)->name = malloc(sizeof(char) *(name_len + 1));
    strncpy((*created_file)->name, filename, name_len);
    (*created_file)->type = type;
    (*created_file)->fileSize = 0;
    (*created_file)->content = NULL;

    if(type == 'd'){
        add_catatlog_record(record_to_find, *created_file);
    }
    if(current_directory != NULL){
        add_simple_record(last_record(record_to_find), *created_file);
    }
    return NO_PROBLEM_FOUND;
}//approved



/*
 * 3 - поиск уже существующих файлов\записей
 */


record* last_record(record *current_record) {
    while (current_record->next != NULL) {
        current_record = current_record->next;
    }
    return current_record;
}//approved

char list_directory_content(file *directory, FILE *fout){
    if(directory->type != 'd'){
        return IS_NOT_A_DIRECTORY;
    }
    else{
        record *records_list = *(record**)directory->content;
        printf("Files in directory: %s\n", records_list[0].current->name);
        do{
            fprintf(fout, "%c %ld %s\n", records_list->current->type, records_list->current->fileSize, records_list->current->name);
            records_list = records_list->next;
        }while(records_list != NULL);
        return NO_PROBLEM_FOUND;
    }
}//approved


char find_record(char **filename, file *current_directory, record **record_pointer){
    char *word = malloc(52 * sizeof(char));
    char state = 1;

    while(**filename){
        *filename = parse_string('\\', *filename, word);
        if(current_directory->type != 'd'){
            free(word);
            return IS_NOT_A_DIRECTORY;
        }
        *record_pointer = *(record**)(current_directory->content);
        if(strcmp(word, "..") == 0){
            current_directory = (*record_pointer)->previous->current;
            if(**filename) ++(*filename);
            else *record_pointer = *(record**)(current_directory->content);
            continue;
        } else{
            while((state = strcmp(word, (*record_pointer)->current->name)) && (*record_pointer)->next != NULL){
                *record_pointer = (*record_pointer)->next;
            }
        }

        if(**filename) ++(*filename);
        if(state){

            if(!(**filename)){
                *filename = word;
                return FILENAME_NOT_FOUND;
            }
            free(word);
            return NO_DIRECTORY_WITH_SUCH_NAME;
        }
        current_directory = (*record_pointer)->current;
    }
    free(word);
    return FILE_ALLREADY_EXISTS;
}//approved


/*
 * 4 - изменение содержимого reg файлов
 */

void rewrite_file(file *regular_file, char *content, size_t content_len){
    regular_file->content = malloc(content_len+1);
    strncpy((char*)regular_file->content, content, content_len);
    regular_file->fileSize = content_len;
}//need to check it

void add_content(file *regular_file, char *content, size_t content_len){
    regular_file->content = realloc(regular_file->content, regular_file->fileSize + content_len + 1);
    strncpy((char*)((record*)(regular_file->content) + regular_file->fileSize), content, content_len);
    regular_file->fileSize = regular_file->fileSize + content_len;
}

/*
 * 5 - удаление, перемещение файлов
 */

void cut_record(record *record_to_delete){
    record_to_delete->previous->next = record_to_delete->next;
    if(record_to_delete->next != NULL){
        record_to_delete->next->previous = record_to_delete->previous;
    }
    free(record_to_delete);
}//approved

char remove_file(char *filename, file *current_directory){
    record *record_list = *(record**)current_directory->content;
    do{
        if(strcmp(record_list->current->name,filename) == 0 ){
            if(record_list->previous != NULL){
                free(record_list->current);
                cut_record(record_list);
                return NO_PROBLEM_FOUND;
            }
            else{
                return CANNOT_REMOVE_DIRECTORY;
            }
        }
        record_list = record_list->next;
    }while(record_list != NULL);
    return FILENAME_NOT_FOUND;
}//need to check it


char navigate(char *filename, file *current_directory, file ** file_pointer){
    record *found_rec;
    char error_code = find_record(&filename, current_directory, &found_rec);
    if(error_code == FILE_ALLREADY_EXISTS){
        if(found_rec->current->type == 'd'){
            *file_pointer = found_rec->current;
            return NO_PROBLEM_FOUND;
        } else {
            return IS_NOT_A_DIRECTORY;
        }
    } else{
        return FILENAME_NOT_FOUND;
    }
}//approved

char move_file(char *res, char *dest, file *current_directory){
    record *record_list = *(record**)current_directory->content;
    char res_set = 0, des_set = 0;
    record *res_file, *dest_file;
    if(strcmp("..", dest) == 0){
        des_set = 1;
        record *temp = *(record**)(current_directory->content);
        dest_file = temp->previous;
    }
    do{
        if(!res_set && strcmp(res, record_list->current->name) == 0){
            res_set = 1;
            res_file = record_list;
        }
        if(!des_set && strcmp(dest, record_list->current->name) == 0){
            des_set = 1;
            dest_file = record_list;
        }
        if(des_set && res_set){
            break;
        }
        record_list = record_list->next;
    }while(record_list != NULL);
    if(dest_file->current->type == 'd' && dest_file != res_file && res_file->current != current_directory){
        add_simple_record(last_record(*(record**)dest_file->current->content), res_file->current);
        cut_record(res_file);
        return NO_PROBLEM_FOUND;

    } else if(!res_set && !des_set){
        return FILENAME_NOT_FOUND;
    }
    else{
        return IS_NOT_A_DIRECTORY;
    }
}

int main(){
    file *home = init_file_system();
    list_directory_content(home,stdout);
    file *res;
    new_file(home, "res", 'd', &res);
    list_directory_content(home,stdout);
    file *input;
    new_file(home, "res\\input", '-', &input);
    list_directory_content(res,stdout);

    return 0;
}

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include "filesystem.h"
#include "errors.h"

int maximumInode = 0;

const char directory_delim = '/';

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
} //do we really need two realisations?!?

/*
 * Две реализации - убрать, если нужно будет использовать код где-то
 */

char *parser_without_terminator(const char delim, char *src, char *dest){
    while(*(src) != delim && *src != 0)
        *(dest++) = *(src++);
    return dest;
} //do we really need two realisations?!?


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
 * имя файла, которое мы хотим создать, тип файла и указатель на указатель на файл
 * В случае успеха - в указатель помещается указатель на новый файл. Возвращается - код ошибки.
 */


char new_file(file *current_directory, char *filename, char type, file **created_file){
    if(!*filename || *filename == '\n') {
        return INVALID_FILE_NAME;
    }
    record *record_to_find;
    char error_code;
    if(current_directory != NULL){
        error_code = find_record(&filename, current_directory, &record_to_find);
        if(error_code != FILENAME_NOT_FOUND) {
            return error_code;
        }
        current_directory = record_to_find->current;
    } else{
        record_to_find = NULL;
    }
    if(!*filename){
        return INVALID_FILE_NAME;
    }
    *created_file = malloc(sizeof(file));
    (*created_file)->inode = maximumInode;
    ++maximumInode;
    (*created_file)->name = malloc(sizeof(char) *(strlen(filename) + 1));
    strcpy((*created_file)->name, filename);
    (*created_file)->type = type;
    (*created_file)->fileSize = 0;
    (*created_file)->content = NULL;
    if(type == 'd'){
        add_catatlog_record(record_to_find, *created_file);
    }
    if(current_directory != NULL){
        add_simple_record(last_record(record_to_find), *created_file);
        ++current_directory->fileSize;
    }
    return NO_PROBLEM_FOUND;
}//approved



/*
 * 3 - поиск уже существующих файлов\записей
 */

char *previous_level(record *current_dir, int *length, char *string){
    current_dir = *(record**)(current_dir->current)->content;
    if(current_dir->current->name[0] != '/'){
        *length += strlen(current_dir->current->name) + 1;
        string = previous_level(current_dir->previous, length, string);
    } else{
        string = malloc(sizeof(char)*(*length + 2));
        *(string++) = '/';
        return string;
    }
    string = parser_without_terminator(directory_delim, current_dir->current->name, string);
    *(string++) = '/';
    return string;

}

char* print_working_directory(file *working_directory){
    char *path_to_directory = NULL;
    int a = 0;
    path_to_directory = previous_level(*(record**)(working_directory->content), &a, path_to_directory);
    *(path_to_directory) = '\0';
    path_to_directory -= a+1;
    return path_to_directory;
}


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
        printf("Type\tSize\tFilename\n");
        do{
            fprintf(fout, "%c\t%ld\t%s\n", records_list->current->type, records_list->current->fileSize, records_list->current->name);
            records_list = records_list->next;
        }while(records_list != NULL);
        return NO_PROBLEM_FOUND;
    }
}//approved

/*
 * @DESCRIPTION:
 * На вход подаются - указатель на массив char'ов (filename), указатель на
 * файл(curent_directory), указатель на указатель на запись(record_pointer)
 *
 * В ходе выполнения ищется указанный файл. Если он найден:
 * возвращается код ошибки FILE_ALLREADY_EXISTS и через record_pointer
 * возврашается запись, с которой указанный файл ассоциируется
 *
 * Если файл не найден, но нам удалось дойти до директории, в которой он должен лежать:
 * возвращается код ошибки FILENAME_NOT_FOUND, имя файла без пути к нему возвращается
 * через filename, а указатель на запись, которая ассоциируется с той директорией, в которой
 * мы ищем файл - возвращается через record_pointer
 *
 * Если по пути мы встретили несуществующую директорию:
 * возвращается код ошибки NO_DIRECTORY_WITH_SUCH_NAME
 *
 * Если filename не является файлом:
 * возвращается код ошибки IS_NOT_A_DIRECTORY
 * (не должно происходить при корректной работе)
 */

file *get_file_parent(record *current_file){
    while(!(current_file->current->type == 'd' && (*(record**)current_file->current->content == current_file))){
        current_file = current_file->previous;
    }
    return current_file->current;
}



char find_record(char **filename, file *current_directory, record **record_pointer){
    char *word = malloc(52 * sizeof(char));
    char state = 1;
    while(**filename){
        *filename = parse_string(directory_delim, *filename, word);
        if(current_directory->type != 'd'){
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
                *record_pointer = *(record**)(current_directory->content);
                return FILENAME_NOT_FOUND;
            }
            return NO_DIRECTORY_WITH_SUCH_NAME;
        }
        current_directory = (*record_pointer)->current;
    }
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
    strncpy((char*)regular_file->content + regular_file->fileSize, content, content_len);
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
    record *found_file;
    char err_code = find_record(&filename, current_directory, &found_file);
    if(err_code == FILE_ALLREADY_EXISTS){
        current_directory = get_file_parent(found_file);
        if(found_file->current->type == '-'){
            free(found_file->current->name);
            free(found_file->current);
            cut_record(found_file);
            --current_directory->fileSize;
            return NO_PROBLEM_FOUND;
        }
        else{
            if(found_file->current->fileSize == 1 && found_file->current->name[0] != '/'){
                free(found_file->current->name);
                free(*(record**)(found_file->current->content));
                free(found_file->current->content);
                free(found_file->current);
                cut_record(found_file);
                --current_directory->fileSize;
                return NO_PROBLEM_FOUND;
            }
            else{
                return DIRECTORY_NOT_EMPTY;
            }

        }
    }
    else{
        return err_code;
    }
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
    record *record_dest, *record_res;
    char error_code = find_record(&res, current_directory, &record_res);
    if(error_code != FILE_ALLREADY_EXISTS){
        printf("First cannot be found. WTF?\n");
        return error_code;
    }
    error_code = find_record(&dest, current_directory, &record_dest);
    if(error_code != FILE_ALLREADY_EXISTS){
        printf("Second cannot be found. WTF?\n");
        return error_code;
    }
    current_directory = get_file_parent(record_res);

    if(record_dest->current != record_res->current && record_dest->current->type == 'd' && record_res->current != get_file_parent(record_dest)){
        --(current_directory->fileSize);
        add_simple_record(last_record(*(record**)record_dest->current->content), record_res->current);
        cut_record(record_res);
        ++(record_dest->current->fileSize);
        return NO_PROBLEM_FOUND;
    } else{
        return INVALID_FILE_NAME;
    }
}
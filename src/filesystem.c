#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include "filesystem.h"
#include "errors.h"

int maximumInode = 0;

record* last_record(record *current_record) {
    while (current_record->next != NULL) {
        current_record = current_record->next;
    }
    return current_record;
}//approved


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

file* new_file(record *currentCatalogRecord, char *filename, char type){
    file *newFile = malloc(sizeof(file));
    newFile->inode = maximumInode;
    ++maximumInode;
    newFile->name = filename;
    newFile->type = type;
    newFile->fileSize = 0;
    newFile->content = NULL;
    if(currentCatalogRecord == NULL){
        add_catatlog_record(currentCatalogRecord, newFile);
    } else{
        if(type == 'd'){
            add_catatlog_record(currentCatalogRecord, newFile);
        }
        add_simple_record(last_record(currentCatalogRecord), newFile);
    }
    return newFile;
}//approved

void rewrite_file(file *regular_file, char *content, size_t content_len){
    regular_file->content = malloc(content_len+1);
    strncpy((char*)regular_file->content, content, content_len);
    regular_file->fileSize = content_len;
}

char list_directory_content(file *directory, FILE *fout){
    if(directory->type != 'd'){
        return IS_NOT_A_DIRECTORY;
    }
    else{
        record *records_list = *(record**)directory->content;
        printf("Files in directory: %s\n", records_list[0].current->name);
        do{
            fprintf(fout, "\t%s: %d %c %d\n", records_list->current->name, records_list->current->inode, records_list->current->type, records_list->current->fileSize);
            records_list = records_list->next;
        }while(records_list != NULL);
        return NO_PROBLEM_FOUND;
    }
}

void cut_record(record *record_to_delete){
    record_to_delete->previous->next = record_to_delete->next;
    if(record_to_delete->next != NULL){
        record_to_delete->next->previous = record_to_delete->previous;
    }
    free(record_to_delete);
}

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
    return FILE_NOT_FOUND;
}

file* navigate(char *filename, file *current_directory){
    file *temp = find(filename, current_directory);
    if(temp->type != 'd'){
        return IS_NOT_A_DIRECTORY;
    } else{
        return temp;
    }
}

file* find(char *filename, file *current_directory){
    if(strcmp(".." , filename) == 0){
        record *temp = *(record**)current_directory->content;
        return temp->previous->current;
    }
    record *record_list = *(record**)current_directory->content;
    while(record_list != NULL){
        if(strcmp(filename, record_list->current->name) == 0){
            return record_list->current;
        }
        record_list = record_list->next;
    }
    return FILE_NOT_FOUND;
}

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
        return FILE_NOT_FOUND;
    }
    else{
        return WRONG_ACTION;
    }

}

void add_content(file *regular_file, char *content, size_t content_len){
    regular_file->content = realloc(regular_file->content, regular_file->fileSize + content_len + 1);
    strncpy((char*)(regular_file->content + regular_file->fileSize), content, content_len);
    regular_file->fileSize = regular_file->fileSize + content_len;
}


file* init_file_system(){
    file *home = new_file(NULL, "/", 'd');
    return home;
}


int main(){
    /*
    file *home = init_file_system();
    file *profile = newFile(*(record**)home->content, "profile", '-');
    file *res = newFile(*(record**)home->content, "res", 'd');
    char hello[] = "Hello, world!";
    char moreHel[] ="\nHalLo";
    reWriteContent(profile, hello, sizeof(hello));
    add_content(profile, moreHel, sizeof(moreHel));

    record *records_list = list_directory_content(home);
    printFileInfo(stdout, records_list);
    move_file("profile", "res", home);
    file *buf = navigate("res", home);
    printFileInfo(stdout, list_directory_content(buf));
    remove_file("profile", buf);
    printFileInfo(stdout, list_directory_content(buf));
     */
    return 0;
}
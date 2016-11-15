#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include "filesystem.h"
#include "errors.h"

int maximumInode = 0;

record* lastRecord(record *currentRecord) {
    while (currentRecord->next != NULL) {
        currentRecord = currentRecord->next;
    }
    return currentRecord;
}

void addFile(record *previous, file *child){
    record *temp = malloc(sizeof(record));
    temp->previous = previous;
    temp->current = child;
    temp->next = NULL;
    previous->next = temp;
    ++previous->current->fileSize;
}

file* newFile(record *currentCatalogRecord, char *filename, char type){
    file *newFile = malloc(sizeof(file));
    newFile->inode = maximumInode;
    ++maximumInode;
    newFile->name = filename;
    newFile->type = type;
    newFile->fileSize = 0;
    newFile->content = NULL;
    if(type == 'd'){
        child = malloc(sizeof(record));
        child->current = newFile;
        (record*)newFile->content = child;
        child->next = NULL;
        ++child->current->fileSize;
        if(currentCatalogRecord == NULL){
            child->previous = child;
            return newFile;
        }
        child->previous = currentCatalogRecord;
    }
    addFile(lastRecord(currentCatalogRecord), child);
    return newFile;
}

void reWriteContent(file *regularFile, char *content, size_t content_len){
    regularFile->content = malloc(content_len+1);
    strncpy((char*)regularFile->content, content, content_len);
    regularFile->fileSize = content_len;
}

record* listDirectoryContent(file *directory){
    if(directory->type != 'd'){
        return IS_NOT_A_DIRECTORY;
    }
    else{
        return *(record**)directory->content;
    }
}

void cutRecord(record *recordToDelete){
    recordToDelete->previous->next = recordToDelete->next;
    if(recordToDelete->next != NULL){
        recordToDelete->next->previous = recordToDelete->previous;
    }
    free(recordToDelete);
}

char removeFile(char *filename, file *currentDirectory){
    record *recordList = listDirectoryContent(currentDirectory);
    do{
        if(strcmp(recordList->current->name,filename) == 0 ){
            if(recordList->previous != NULL){
                free(recordList->current);
                cutRecord(recordList);
                return NO_PROBLEM_FOUND;
            }
            else{
                return CANNOT_REMOVE_DIRECTORY;
            }
        }
        recordList = recordList->next;
    }while(recordList != NULL);
    return FILE_NOT_FOUND;
}

file* navigate(char *filename, file *currentDirectory){
    file *temp = find(filename, currentDirectory);
    if(temp->type != 'd'){
        return IS_NOT_A_DIRECTORY;
    } else{
        return temp;
    }
}

file* find(char *filename, file *currentDirectory){
    printf("INPUT\n");
    if(strcmp(".." , filename) == 0){
        currentDirectory = currentDirectory->parent;
        return currentDirectory;
    }
    printf("INPUT  %p\n", currentDirectory);
    record *recordList = listDirectoryContent(currentDirectory);
    while(recordList != NULL){
        printf("HERE: \n");
        if(strcmp(filename, recordList->current->name) == 0){
            return recordList->current;
        }
        recordList = recordList->next;
    }
    printf("Not OK\n");
    return FILE_NOT_FOUND;
}

char moveFile(char *res, char *dest, file *currentDirectory){
    record *recordList = listDirectoryContent(currentDirectory);
    char resIsSet = 0, destIsSet = 0;
    record *res_file, *dest_file;
    //file *res_file, *dest_file;
    if(strcmp("..", dest) == 0){
        destIsSet = 1;
        dest_file->current = currentDirectory->parent;
    }
    do{
        if(!resIsSet && strcmp(res, recordList->current->name) == 0){
            resIsSet = 1;
            res_file = recordList;
        }
        if(!destIsSet && strcmp(dest, recordList->current->name) == 0){
            destIsSet = 1;
            dest_file = recordList;
        }
        if(destIsSet && resIsSet){
            break;
        }
        recordList = recordList->next;
    }while(recordList != NULL);
    if(dest_file->current->type == 'd' && dest_file != res_file && res_file->current != currentDirectory){
        res_file->current->parent = dest_file;
        addFile(dest_file->current, res_file->current, lastRecord(dest_file->current));
        cutRecord(res_file);
        return NO_PROBLEM_FOUND;

    } else if(!resIsSet && !destIsSet){
        return FILE_NOT_FOUND;
    }
    else{
        return WRONG_ACTION;
    }

}

char printFileInfo(FILE* fout, record *recordsList){
    printf("Files in directory: %s\n", recordsList[0].current->name);
    do{
        fprintf(fout, "\t%s: %d %c %d\n", recordsList->current->name, recordsList->current->inode, recordsList->current->type, recordsList->current->fileSize);
        recordsList = recordsList->next;
    }while(recordsList != NULL);
}

void addContent(file *regularFile, char *content, size_t content_len){
    regularFile->content = realloc(regularFile->content, regularFile->fileSize + content_len + 1);
    strncpy((char*)(regularFile->content + regularFile->fileSize), content, content_len);
    regularFile->fileSize = regularFile->fileSize + content_len;
}


file* initFileSystem(){
    file *home = newFile(NULL, "/", 'd', NULL);
    home->parent = home;
    return home;
}


int main(){
    file *home = initFileSystem();
    file *profile = newFile(home, "profile", '-',lastRecord(home));
    file *res = newFile(home, "res", 'd', lastRecord(home));
    printf("Content added: ");
    fflush(stdout);
    char input[] = "Hello, world!";
    char moreHel[] ="\nHalLo";
    reWriteContent(profile, hello, sizeof(hello));
    addContent(profile, moreHel, sizeof(moreHel));
    record *recordsList = listDirectoryContent(home);
    printFileInfo(stdout, recordsList);
    moveFile("profile", "res", home);
    file *buf = navigate("res", home);
    printFileInfo(stdout, listDirectoryContent(buf));
    removeFile("profile", buf);
    printFileInfo(stdout, listDirectoryContent(buf));
    return 0;
}
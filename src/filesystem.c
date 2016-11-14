#include "filesystem.h"
#include "errors.h"


void addFile(file *parent, file *child, record* previous){
    if(parent->actualSize == parent->usedSize){
        parent->actualSize += DEFAULT_INCREASE;
        parent->content = realloc(parent->content, parent->actualSize * sizeof(record));
    }
    record *temp = malloc(sizeof(record));
    temp->previous = previous;
    temp->current = child;
    temp->next = NULL;
    record **current = (record**)(parent->content + sizeof(record)*parent->usedSize);
    *current = temp;
    if(previous != NULL){
        previous->next = temp;
    }
    printf("Add new: %d %p\n", parent->usedSize, (record*)(parent->content + parent->usedSize * sizeof(record)));
    ++parent->usedSize;
}

void reWriteContent(file *regularFile, char *content, size_t content_len){
    regularFile->content = malloc(content_len+1);
    strncpy((char*)regularFile->content, content, content_len);
    regularFile->actualSize = regularFile->usedSize = content_len;
}

record* listDirectoryContent(file *directory){
    if(directory->type != 'd'){
        return IS_NOT_A_DIRECTORY;
    }
    else{
        return *(record**)directory->content;
    }
}

char removeFile(char *filename, file *currentDirectory){
    record *recordList = listDirectoryContent(currentDirectory);
    do{
        if(strcmp(recordList->current->name,filename) == 0 ){
            if(recordList->previous != NULL){
                recordList->previous->next = recordList->next;
                if(recordList->next != NULL){
                    recordList->next->previous = recordList->previous;
                }
                free(recordList->current);
                free(recordList);
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
    if(strcmp(".." , filename) == 0){
        currentDirectory = currentDirectory->parent;
        return currentDirectory;
    }
    record *recordList = listDirectoryContent(currentDirectory);
    do{
        if(strcmp(filename, recordList->current->name) == 0){
            if(recordList->current->type == 'd'){
                return recordList->current;
            }
            else{
                return IS_NOT_A_DIRECTORY;
            }
        }
        recordList = recordList->next;
    }while(recordList != NULL);
    return FILE_NOT_FOUND;

}

char printFileInfo(FILE* fout, record *recordsList){
    printf("Files in directory: %s\n", recordsList[0].current->name);
    do{
        fprintf(fout, "\t%s: %d %c %d\n", recordsList->current->name, recordsList->current->inode, recordsList->current->type, recordsList->current->usedSize);
        recordsList = recordsList->next;
    }while(recordsList != NULL);
}

void addContent(file *regularFile, char *content, size_t content_len){
    regularFile->content = realloc(regularFile->content, regularFile->actualSize + content_len + 1);
    strncpy((char*)(regularFile->content + regularFile->actualSize - 1), content, content_len);
    regularFile->actualSize = regularFile->usedSize = regularFile->actualSize + content_len;
}

record* lastRecord(file *directory){
    return  *(record**)(directory->content + (directory->usedSize - 1) * sizeof(record));
}


file* newFile(file *parent, char *filename, char type, record *prevRecord){
    file *newFile = malloc(sizeof(file));
    newFile->parent = parent;
    newFile->inode = maxInode;
    ++maxInode;
    newFile->name = filename;
    newFile->type = type;
    newFile->actualSize = 0;
    newFile->usedSize = 0;
    newFile->content = NULL;
    if(parent != NULL){
        addFile(parent, newFile, prevRecord);
        if(type == 'd') addFile(newFile, newFile, NULL);
        parent = newFile;
    }
    else{
        addFile(newFile, newFile, NULL);
    }

    return newFile;
}


file* initFileSystem(){
    file *home = newFile(NULL, "home", 'd', NULL);
    home->parent = home;
    return home;
}


int main(){
    file *home = initFileSystem();
    file *profile = newFile(home, "profile", '-',lastRecord(home));
    file *res = newFile(home, "res", 'd', lastRecord(home));
    char hello[] = "Hello, world!";
    char moreHel[] ="\nHalLo";
    reWriteContent(profile, hello, sizeof(hello));
    addContent(profile, moreHel, sizeof(moreHel));
    record *recordsList = listDirectoryContent(home);
    printFileInfo(stdout, recordsList);
    file *buf = navigate("res", home);
    printFileInfo(stdout, listDirectoryContent(buf));
    return 0;
}

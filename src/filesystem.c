#include "filesystem.h"
#include "errors.h"


record* lastRecord(file *directory){
    return  *(record**)(directory->content + (directory->usedSize - 1) * sizeof(record));
}

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
        fprintf(fout, "\t%s: %d %c %d\n", recordsList->current->name, recordsList->current->inode, recordsList->current->type, recordsList->current->usedSize);
        recordsList = recordsList->next;
    }while(recordsList != NULL);
}

void addContent(file *regularFile, char *content, size_t content_len){
    regularFile->content = realloc(regularFile->content, regularFile->actualSize + content_len + 1);
    strncpy((char*)(regularFile->content + regularFile->actualSize - 1), content, content_len);
    regularFile->actualSize = regularFile->usedSize = regularFile->actualSize + content_len;
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

/*
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
    moveFile("profile", "res", home);
    file *buf = navigate("res", home);
    printFileInfo(stdout, listDirectoryContent(buf));
    removeFile("profile", buf);
    printFileInfo(stdout, listDirectoryContent(buf));
    return 0;
}
 */
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

record** listDirectoryContent(file *directory){
    if(directory->type != 'd'){
        return IS_NOT_A_DIRECTORY;
    }
    else{
        return *(record**)directory->content;
    }
}

char printFileInfo(FILE* fout, record *recordsList){
    printf("Files in directory: %s\n", recordsList[0].current->name);
    do{
        fflush(stdout);
        fprintf(fout, "%s: %d %c %d\n", recordsList->current->name, recordsList->current->inode, recordsList->current->type, recordsList->current->usedSize);
        recordsList = recordsList->next;
    }while(recordsList != NULL);
    //
}

void addContent(file *regularFile, char *content, size_t content_len){
    regularFile->content = realloc(regularFile->content, regularFile->actualSize + content_len + 1);
    strncpy((char*)(regularFile->content + regularFile->actualSize - 1), content, content_len);
    regularFile->actualSize = regularFile->usedSize = regularFile->actualSize + content_len;
}


file* newFile(file *parent, char *filename, char type, record *prevRecord){
    file *newFile = malloc(sizeof(file));
    if(parent == NULL){
        parent = newFile;
    }
    newFile->parent = parent;
    newFile->inode = maxInode;
    ++maxInode;
    newFile->name = filename;
    newFile->type = type;
    newFile->actualSize = 0;
    newFile->usedSize = 0;
    newFile->content = NULL;
    addFile(parent, newFile, prevRecord);
    return newFile;
}


file* initFileSystem(){
    file *home = newFile(NULL, "home", 'd', NULL);
    home->parent = home;
    return home;
}


int main(){
    file *home = initFileSystem();
    file *profile = newFile(home, "profile", '-', *(record**)(home->content + (home->usedSize - 1) * sizeof(record)));
    char hello[] = "Hello, world!";
    char moreHel[] ="\nHalLo";
    reWriteContent(profile, hello, sizeof(hello));
    addContent(profile, moreHel, sizeof(moreHel));
    record *recordsList = listDirectoryContent(home);
    printFileInfo(stdout, recordsList);
    return 0;
}

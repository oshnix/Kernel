#include "filesystem.h"

void addFile(file *parent, file *child){
    if(parent->actualSize == parent->usedSize){
        parent->actualSize += DEFAULT_INCREASE * sizeof(file);
        parent->content = realloc(parent->content, parent->actualSize);
    }
    parent->content[parent->usedSize] = child;
    parent->usedSize += sizeof(void*);
}


file newFile(file *parent, char *dirname, int newInode, char type){
    file directory = {parent, newInode, dirname, type, 0, 0, NULL};
    if(parent != NULL){
        addFile(parent, &directory);
    }
    return directory;
}


void initFileSystem(){
    file home = addFile(NULL, "home", 0, 'd');
    home.parent = &home;
}

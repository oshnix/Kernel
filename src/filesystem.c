#include "filesystem.h"

void addFile(file *parent, file *child){
    if(parent->actualSize == parent->usedSize){
        parent->actualSize += DEFAULT_INCREASE;
        parent->content = realloc(parent->content, parent->actualSize * sizeof(file));
    }
    *((file**)(parent->content + parent->usedSize)) = child;
    ++parent->usedSize;
}


file newFile(file *parent, char *filename, char type){
    file newFile = {parent, maxInode, filename, type, 0, 0, NULL};
    ++maxInode;
    if(parent != NULL){
        addFile(parent, &newFile);
    }
    return newFile;
}


file initFileSystem(){
    file home = newFile(NULL, "home", 'd');
    home.parent = &home;
    return home;
}


int main(){
    file home = initFileSystem();
    file profile = newFile(&home, "profile", '-');
    file *prof = *(file**)home.content;
    printf("%p %d %d %d\n",prof->parent, prof->inode, prof->actualSize, prof->usedSize);
    return 0;
}

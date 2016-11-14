#include "filesystem.h"


void addFile(file *parent, file *child){
    if(parent->actualSize == parent->usedSize){
        parent->actualSize += DEFAULT_INCREASE;
        parent->content = realloc(parent->content, parent->actualSize);
    }
    *((file**)(parent->content + parent->usedSize)) = child;
    ++parent->usedSize;
}

void reWriteContent(file *regularFile, char *content, size_t content_len){
    regularFile->content = malloc(content_len+1);
    strncpy((char*)regularFile->content, content, content_len);
    regularFile->actualSize = regularFile->usedSize = content_len;
}

void addContent(file *regularFile, char *content, size_t content_len){
    regularFile->content = realloc(regularFile->content, regularFile->actualSize + content_len + 1);
    strncpy((char*)(regularFile->content + regularFile->actualSize - 1), content, content_len);
    regularFile->actualSize = regularFile->usedSize = regularFile->actualSize + content_len;
}


file* newFile(file *parent, char *filename, char type){
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
        addFile(parent, newFile);
    }
    return newFile;
}


file* initFileSystem(){
    file *home = newFile(NULL, "home", 'd');
    home->parent = home;
    return home;
}


int main(){
    file *home = initFileSystem();
    file *profile = newFile(home, "profile", '-');
    char hello[] = "Hello, world!";
    char moreHel[] ="\nHalLo";
    file *prof = *(file**)home->content;
    printf("Prof: %d %d %d %s\n", prof->inode, prof->actualSize, prof->usedSize, (char*)prof->content);
    reWriteContent(prof, hello, sizeof(hello));
    addContent(profile, moreHel, sizeof(moreHel));
    printf("Prof: %d %d %d %s\n", profile->inode, profile->actualSize, profile->usedSize, (char*)profile->content);
    return 0;
}

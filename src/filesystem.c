#include "filesystem.h"
#include "errors.h"


void addFile(file *parent, file *child){
    if(parent->actualSize == parent->usedSize){
        parent->actualSize += DEFAULT_INCREASE;
        parent->content = realloc(parent->content, parent->actualSize * sizeof(record));
    }
    *((file**)(parent->content + sizeof(void*)*parent->usedSize)) = child;
    printf("Add new: %p %p\n", parent, child);
    ++parent->usedSize;
}

void reWriteContent(file *regularFile, char *content, size_t content_len){
    regularFile->content = malloc(content_len+1);
    strncpy((char*)regularFile->content, content, content_len);
    regularFile->actualSize = regularFile->usedSize = content_len;
}

file** listDirectoryContent(file *directory, int *size){
    if(directory->type != 'd'){
        return IS_NOT_A_DIRECTORY;
    }
    else{
        *size = directory->usedSize;
        return (file**)directory->content;
    }
}

char printFileInfo(FILE* fout, file* fileToPrint){
    fprintf(fout, "%s: %d %c %d\n", fileToPrint->name, fileToPrint->inode, fileToPrint->type, fileToPrint->usedSize);

}

void addContent(file *regularFile, char *content, size_t content_len){
    regularFile->content = realloc(regularFile->content, regularFile->actualSize + content_len + 1);
    strncpy((char*)(regularFile->content + regularFile->actualSize - 1), content, content_len);
    regularFile->actualSize = regularFile->usedSize = regularFile->actualSize + content_len;
}


file* newFile(file *parent, char *filename, char type, file* prevFile){
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
    addFile(parent, newFile);
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
    file *prof = *(file**)(home->content + sizeof(void*));
    reWriteContent(profile, hello, sizeof(hello));
    addContent(profile, moreHel, sizeof(moreHel));
    int count;
    file **fileList = listDirectoryContent(home, &count);
    printf("Files in directory: %s\n", fileList[0]->name);
    for(int i = 0; i < count; i++){
        printFileInfo(stdout, fileList[i]);
        fflush(stdout);
    }
    return 0;
}

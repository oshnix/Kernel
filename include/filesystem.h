#ifndef _FILESYSTEM_H_
#define _FILESYSTEM_H_

#pragma pack(push, 2)
typedef struct file{
    int inode;
    char *name;
    char type;
    size_t fileSize;
    void *content;
}file;
#pragma pack(pop)


typedef struct record{
    struct record *previous;
    struct record *next;
     file  *current;
}record;



record* lastRecord(record *currentRecord);
//принимает указатель на запись в директории и ищет последнюю
void add_simple_record(record *previous, file *child);
//добавляет новую запись к концу списка
void add_catatlog_record(record *parent_directory_record, file *new_directory);
//
char listDirectoryContent(file *directory);
//если файл является директорией, то возвращает запись, которая указывает на саму эту директорию
void cutRecord(record *recordToDelete);
//вырезает запись из списка
char removeFile(char *filename, file *currentDirectory);
//удаляет запись о файле и его содержимое
file* navigate(char *filename, file *currentDirectory);
//возвращает файл, если в текущей директории был найден файл с таким названием.
void reWriteContent(file *regularFile, char *content, size_t content_len);
char moveFile(char *res, char *dest, file *currentDirectory);
void addContent(file *regularFile, char *content, size_t content_len);
file* newFile(record *currentCatalogRecord, char *filename, char type);
file* find(char *filename, file *currentDirectory);
file* initFileSystem();


#endif

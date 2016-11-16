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

char *parse_string(const char delim, char *src, char *dest);




record* last_record(record *current_record);

void add_simple_record(record *previous, file *child);

void add_catatlog_record(record *parent_directory_record, file *new_directory);

file* new_file(record *currentCatalogRecord, char *filename, char type, size_t filename_length );

void rewrite_file(file *regularFile, char *content, size_t content_len);

char list_directory_content(file *directory, FILE *fout);

void cutRecord(record *record_to_delete);

char remove_file(char *filename, file *current_directory);

file* find(char *filename, file *current_directory);

char find_record(char *filename, file *current_directory, record **record_pointer);

char navigate(char *filename, file *current_directory, file ** file_pointer);

char move_file(char *res, char *dest, file *current_directory);

void add_content(file *regularFile, char *content, size_t content_len);

file* init_file_system();


#endif

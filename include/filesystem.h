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

//1

char *parse_string(const char delim, char *src, char *dest);

//2

file* init_file_system();

char new_file(file *current_directory, char *filename, char type, file **created_file);

//3

char* print_working_directory(file *working_directory);

record* last_record(record *current_record);

char list_directory_content(file *directory, FILE *fout);

char find_record(char **filename, file *current_directory, record **record_pointer);

//4

void rewrite_file(file *regularFile, char *content, size_t content_len);
void add_content(file *regularFile, char *content, size_t content_len);

//5

char remove_file(char *filename, file *current_directory);

char navigate(char *filename, file *current_directory, file ** file_pointer);

char move_file(char *res, char *dest, file *current_directory);



#endif

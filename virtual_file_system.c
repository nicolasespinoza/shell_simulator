#include <stdio.h>
#include <stdlib.h>
#include "virtual_file_system.h"

struct virtual_file_system* create_virtual_file_system() {
    struct virtual_file_system* virtualFileSystem;
    virtualFileSystem = malloc(sizeof(struct virtual_file_system));

    return virtualFileSystem;
}

// first parameter for all these functions will be either a directory "object" of some sort or an inode
void change_directory(struct virtual_file_system* virtualFileSystem, char* directory_name) {
    printf("Change to directory [%s]\n", directory_name);
}

void list_directory(struct virtual_file_system* virtualFileSystem) {
    printf("List directory\n");
}

void make_directory(struct virtual_file_system* virtualFileSystem, char* directory_name) {
    printf("Create directory [%s]\n", directory_name);
}

void touch_file(struct virtual_file_system* virtualFileSystem, char* file_name) {
    printf("Touch file [%s]\n", file_name);
}
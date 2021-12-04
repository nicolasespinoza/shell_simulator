#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include "virtual_file_system.h"

struct virtual_file_system* create_virtual_file_system() {
    struct virtual_file_system* virtualFileSystem;
    virtualFileSystem = malloc(sizeof(struct virtual_file_system));

    return virtualFileSystem;
}

void parse_files_from_directory(struct virtual_file_system* virtualFileSystem, char* directory_name) {
//    DIR* directory = opendir(directory_name);
//    if (directory) {
//        printf("DIrectory exxists");
//        char* inodes_path = directory_name + "/inodes_list";
//        if (access(directory_name + "/inodes_list", F_OK) == 0) {
//            printf("inodes_list found");
//        } else {
//            printf("There was a problem finding the inodes_list file");
//            exit(1);
//        }
//        closedir(directory);
//    } else {
//        printf("There was a problem opening directory %s (does it exist?).\n", directory_name);
//        exit(1);
//    }
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
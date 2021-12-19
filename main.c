#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util/array_list.h"
#include "util/toolkit.h"
#include "emulator.h"

struct inode* new_inode(char* index, char* type) {
    struct inode* node = malloc(sizeof(struct inode));
    node->index = index;
    node->type = type;
    return node;
}

// make helper function clear inodes_list arraylist with node items inside. You just need to free the "type" part of
// the struct because that uses strdup

struct inode* inode_from_line(char* line, int line_number, int max_number_of_inodes) {
    struct arraylist* split_data = split(line, " ");

    char* inode_index = strdup(array_list_get_item(split_data, 0));
    char* inode_type = strdup(array_list_get_item(split_data, 1));
    inode_type[strcspn(inode_type, "\n")] = 0; // remove trailing newline character

    if (string_to_int(inode_index) < 0) {
        printf("inode on line %d has invalid index [%s], skipping...\n", line_number, inode_index);
        return NULL;
    }

    if (string_to_int(inode_index) > max_number_of_inodes) {
        printf("inode on line %d has invalid index [%s] above the max allowed [%d], skipping...\n", line_number,
               inode_index, max_number_of_inodes);
        return NULL;
    }

    if (strcmp(inode_type, "f") != 0 && strcmp(inode_type, "d") != 0) {
        printf("inode on line %d has invalid type [%s], skipping...\n", line_number, inode_type);
        return NULL;
    }

    array_list_cleanup(split_data);
    return new_inode(inode_index, inode_type);
}

struct arraylist* parse_inodes_list(char* file_system_directory, int max_number_of_inodes) {
    char inodes_file[50] = "";
    strcat(inodes_file, file_system_directory);
    strcat(inodes_file, "/inodes_list");

    struct arraylist* inodes_list = array_list_new(sizeof(struct inode));
    FILE* file = fopen(inodes_file, "r");

    char* line_buffer = NULL;
    size_t line_buffer_size = 50;
    int line_number = 0;

    while (getline(&line_buffer, &line_buffer_size, file) > 0) {
        line_number++;

        if (strlen(line_buffer) == 1) { // ignore blank lines
            continue;
        }

        struct inode* node = inode_from_line(line_buffer, line_number, max_number_of_inodes);
        if (node != NULL) {
            array_list_add_to_end(inodes_list, node);
        }
    }

    free(line_buffer);
    fclose(file);
    return inodes_list;
}

void emulator_cleanup(struct emulator* emulator) {
    // Clean up inodes_list
    struct arraylist* inodes_list = emulator->inodes_list;
    for (int i = 0; i < inodes_list->number_of_items; i++) {
        struct inode* node = array_list_get_item(inodes_list, i);
        free(node->index);
        free(node->type);
    }
    array_list_cleanup(inodes_list);

    free(emulator->current_directory_inode_index);
    free(emulator);
}

int main(int number_of_arguments, char* arguments[]) {
    if (number_of_arguments == 3) {
        int max_number_of_inodes = string_to_int(arguments[1]);
        char* file_system_directory = validate_file_exists(arguments[2]);

        struct emulator* emulator = emulator_object_create(file_system_directory, max_number_of_inodes);
        emulator->total_supported_nodes = max_number_of_inodes;
        emulator->file_system_directory = file_system_directory;
        emulator->inodes_list = parse_inodes_list(file_system_directory, max_number_of_inodes);

        emulate_shell(emulator);
        emulator_cleanup(emulator);
    } else {
        printf("Syntax: ./a.out <max number of inodes> <file system directory>\n");
    }
    return 0;
}

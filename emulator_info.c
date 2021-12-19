#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "util/toolkit.h"
#include "util/array_list.h"
#include "emulator_info.h"

struct emulator_info* emulator_object_create(struct arraylist* inodes_list, char* file_system_directory, int total_supported_nodes) {
    struct emulator_info* emulator = malloc(sizeof(struct emulator_info));
    emulator->inodes_list = inodes_list;
    emulator->file_system_directory = file_system_directory;
    emulator->total_supported_nodes = total_supported_nodes;
    emulator->current_directory_inode_index = strdup("0");
    return emulator;
}

struct inode* new_inode(char* index, char* type) {
    struct inode* node = malloc(sizeof(struct inode));
    node->index = index;
    node->type = type;
    return node;
}

struct file_node* create_file_node(char* file_name, char* index) {
    struct file_node *file_node = malloc(sizeof(struct file_node));
    file_node->file_name = strdup(file_name);
    file_node->index = strdup(index);
    return file_node;
}

void file_node_array_list_cleanup(struct arraylist* list) {
    for (int i = 0; i < list->number_of_items; i++) {
        struct file_node* file_node = array_list_get_item(list, i);
        free(file_node->index);
        free(file_node->file_name);
    }
    array_list_cleanup(list);
}

struct arraylist* get_directory_contents(struct emulator_info* emulator, char* inode_index) {
    struct arraylist* directory_contents = array_list_new(sizeof(struct file_node));

    char inode_file_path[50] = "";
    strcat(inode_file_path, emulator->file_system_directory);
    strcat(inode_file_path, "/");
    strcat(inode_file_path, inode_index);

    FILE* inode_file = fopen(inode_file_path, "r");

    char* line_buffer = NULL;
    size_t line_buffer_size = 50;

    while (getline(&line_buffer, &line_buffer_size, inode_file) > 0) {
        if (strlen(line_buffer) == 1) { // ignore blank lines
            continue;
        }

        line_buffer[strcspn(line_buffer, "\n")] = 0; // remove trailing newline character

        struct arraylist* split_data = split(line_buffer, " ");
        char* current_file_index = array_list_get_item(split_data, 0);
        char* current_file_name = array_list_get_item(split_data, 1);

        array_list_add_to_end(directory_contents, create_file_node(current_file_name, current_file_index));
        array_list_cleanup(split_data);
    }

    free(line_buffer);
    fclose(inode_file);

    return directory_contents;
}

int is_directory(struct emulator_info* emulator, char* index) {
    for (int i = 0; i < emulator->inodes_list->number_of_items; i++) {
        struct inode* node = array_list_get_item(emulator->inodes_list, i);
        if (strcmp(node->index, index) == 0 && strcmp(node->type, "d") == 0) {
            return 1;
        }
    }
    return 0;
}

char* file_name_to_index_from_current_directory(struct emulator_info* emulator, char* file_name, int only_directories) {
    struct arraylist* directory_contents = get_directory_contents(emulator, emulator->current_directory_inode_index);

    for (int i = 0; i < directory_contents->number_of_items; i++) {
        struct file_node* file_node = array_list_get_item(directory_contents, i);
        if (strcmp(file_node->file_name, file_name) == 0) {
            if (only_directories && !is_directory(emulator, file_node->index)) {
                file_node_array_list_cleanup(directory_contents);
                return "-2";
            }
            char* file_node_index = strdup(file_node->index);
            file_node_array_list_cleanup(directory_contents);
            return file_node_index;
        }
    }
    file_node_array_list_cleanup(directory_contents);
    return "-1";
}

int file_exists(struct emulator_info* emulator, char* file_name) {
    char* if_existing_file_index = file_name_to_index_from_current_directory(emulator, file_name, 0);
    if (strcmp(if_existing_file_index, "-1") != 0 && strcmp(if_existing_file_index, "-2") != 0) {
        int result = strcmp(if_existing_file_index, "-1") != 0;
        free(if_existing_file_index);
        return result;
    }
    return strcmp(if_existing_file_index, "-1") != 0;
}

struct arraylist* get_all_currently_used_inode_indexes(struct emulator_info* emulator) {
    struct arraylist* currently_used_inode_indexes = array_list_new(sizeof(char*));
    struct arraylist* inodes_list = emulator->inodes_list;

    for (int i = 0; i < inodes_list->number_of_items; i++) {
        struct inode* node = array_list_get_item(emulator->inodes_list, i);
        if (node != NULL) {
            array_list_add_to_end(currently_used_inode_indexes, strdup(node->index));
        }
    }

    return currently_used_inode_indexes;
}

int list_contains_str(struct arraylist* list, char* string) {
    for (int i = 0; i < list->number_of_items; i++) {
        char* item = array_list_get_item(list, i);
        if (strcmp(item, string) == 0) {
            return 1;
        }
    }
    return 0;
}

char* get_available_inode_index(struct emulator_info* emulator, char* type) {
    struct arraylist* inodes_list = emulator->inodes_list;
    for (int i = 0; i < inodes_list->number_of_items; i++) {
        struct inode* node = array_list_get_item(inodes_list, i);
        if (strcmp(node->type, "N") == 0) {
            free(node->type);
            node->type = strdup(type); // previously deleted inode will not be used
            return strdup(node->index);
        }
    }

    struct arraylist* currently_used_inode_indexes = get_all_currently_used_inode_indexes(emulator);
    for (int i = 0; i < emulator->total_supported_nodes + 1; i++) {
        char index_char[20] = "";
        sprintf(index_char, "%d", i);

        if (!list_contains_str(currently_used_inode_indexes, index_char)) {
            char* index_found = strdup(index_char);
            array_list_cleanup(currently_used_inode_indexes);
            array_list_add_to_end(emulator->inodes_list, new_inode(strdup(index_found), strdup(type)));
            return index_found;
        }
    }
    array_list_cleanup(currently_used_inode_indexes);
    printf("operation failed: max inode limit reached\n");
    return "-1";
}

char* to_inode_data_format(char* index, char* type) {
    char inode_data_line[50] = "";
    strcat(inode_data_line, index);
    strcat(inode_data_line, " ");
    strcat(inode_data_line, type);
    strcat(inode_data_line, "\n");

    return strdup(inode_data_line);
}

char* to_directory_data_format(char* index, char* file_name) {
    char directory_data_line[50] = "";
    strcat(directory_data_line, index);
    strcat(directory_data_line, " ");
    strcat(directory_data_line, file_name);
    strcat(directory_data_line, "\n");

    return strdup(directory_data_line);
}

char* get_inodes_list_path(struct emulator_info* emulator) {
    char inodes_list_file_name[50] = "";
    strcat(inodes_list_file_name, emulator->file_system_directory);
    strcat(inodes_list_file_name, "/");
    strcat(inodes_list_file_name, "inodes_list");

    return strdup(inodes_list_file_name);
}

char* get_directory_data_path(struct emulator_info* emulator, char* index) {
    char directory_file_path[50] = "";
    strcat(directory_file_path, emulator->file_system_directory);
    strcat(directory_file_path, "/");
    strcat(directory_file_path, index);

    return strdup(directory_file_path);
}

void append_to_directory_data(struct emulator_info* emulator, char* index, char* file_name) {
    char* directory_file_path = get_directory_data_path(emulator, emulator->current_directory_inode_index);
    char* directory_data_line = to_directory_data_format(index, file_name);

    FILE* directory_data = fopen(directory_file_path, "a");
    fwrite(directory_data_line, 1, strlen(directory_data_line), directory_data);

    free(directory_file_path);
    free(directory_data_line);
    fclose(directory_data);
}

void append_inodes_list(struct emulator_info* emulator, char* index, char* type) {
    char* inodes_list_file_name = get_inodes_list_path(emulator);
    char* inode_data_line = to_inode_data_format(index, type);

    FILE* inodes_list_file = fopen(inodes_list_file_name, "a");
    fwrite(inode_data_line, 1, strlen(inode_data_line), inodes_list_file);

    free(inode_data_line);
    free(inodes_list_file_name);
    fclose(inodes_list_file);
}

void create_data_file(struct emulator_info* emulator, char* index, char* type, char* if_file_name) {
    char new_data_file_path[50] = "";
    strcat(new_data_file_path, emulator->file_system_directory);
    strcat(new_data_file_path, "/");
    strcat(new_data_file_path, index);

    FILE *new_data_file = fopen(new_data_file_path, "wb");

    if (strcmp(type, "f") == 0) {
        fwrite(if_file_name, 1, strlen(if_file_name), new_data_file);
    } else if (strcmp(type, "d") == 0) {
        char directory_local[50] = "";
        strcat(directory_local, index);
        strcat(directory_local, " .\n");

        char directory_above[50] = "";
        strcat(directory_above, emulator->current_directory_inode_index);
        strcat(directory_above, " ..\n");

        fwrite(directory_local, 1, strlen(directory_local), new_data_file);
        fwrite(directory_above, 1, strlen(directory_above), new_data_file);
    }

    fclose(new_data_file);
}

void remove_line_x_from_file(char* file_name, char* line_to_remove) {
    FILE* original_file = fopen(file_name, "r");

    char file_new_copy_path[50] = "";
    strcat(file_new_copy_path, file_name);
    strcat(file_new_copy_path, "_temp_emu");

    FILE* file_new_copy = fopen(file_new_copy_path, "wb");

    char* line_buffer = NULL;
    size_t line_buffer_size = 50;

    while (getline(&line_buffer, &line_buffer_size, original_file) > 0) {
        if (strlen(line_buffer) == 1) { // ignore blank lines
            continue;
        }

        if (strcmp(line_buffer, line_to_remove) == 0) {
            continue; // do not write the line to remove to the new file
        }

        fwrite(line_buffer, 1, strlen(line_buffer), file_new_copy);
    }
    free(line_buffer);

    fclose(original_file);
    fclose(file_new_copy);

    remove(file_name); // deletes original with undesired line
    rename(file_new_copy_path, file_name);
}

void change_directory(struct emulator_info* emulator, char* to_directory_name) {
    char* to_directory_index = file_name_to_index_from_current_directory(emulator, to_directory_name, 1);

    if (strcmp(to_directory_index, "-1") == 0) {
        printf("cd: no such file or directory: %s\n", to_directory_name);
        return;
    } else if (strcmp(to_directory_index, "-2") == 0) {
        printf("cd: not a directory: %s\n", to_directory_name);
        return;
    }

    free(emulator->current_directory_inode_index);
    emulator->current_directory_inode_index = to_directory_index;
}

void list_directory(struct emulator_info* emulator) {
    struct arraylist* current_directory_contents =
            get_directory_contents(emulator, emulator->current_directory_inode_index);
    for (int i = 0; i < current_directory_contents->number_of_items; i++) {
        struct file_node* file_node = array_list_get_item(current_directory_contents, i);
        printf("%s %s\n", file_node->index, file_node->file_name);
    }
    file_node_array_list_cleanup(current_directory_contents);
}

void create_directory(struct emulator_info* emulator, char* file_name) {
    if (file_exists(emulator, file_name)) {
        printf("mkdir: %s: File exists\n", file_name);
        return;
    }

    char* next_available_inode_index = get_available_inode_index(emulator, "d");
    if (strcmp(next_available_inode_index, "-1") == 0) {
        return;
    }

    append_to_directory_data(emulator, next_available_inode_index, file_name);
    append_inodes_list(emulator, next_available_inode_index, "d");
    create_data_file(emulator, next_available_inode_index, "d", NULL);

    free(next_available_inode_index);
}

void touch_file(struct emulator_info* emulator, char* file_name) {
    if (file_exists(emulator, file_name)) {
        return;
    }

    char* next_available_inode_index = get_available_inode_index(emulator, "f");
    if (strcmp(next_available_inode_index, "-1") == 0) {
        return;
    }

    append_to_directory_data(emulator, next_available_inode_index, file_name); // adds to current directory file
    append_inodes_list(emulator, next_available_inode_index, "f");
    create_data_file(emulator, next_available_inode_index, "f", file_name);

    free(next_available_inode_index);
}

void remove_file_or_directory(struct emulator_info* emulator, char* file_name) {
    if (!file_exists(emulator, file_name)) {
        printf("rm: %s: No such file or directory\n", file_name);
        return;
    }

    char* file_to_remove_index = file_name_to_index_from_current_directory(emulator, file_name, 0);
    int is_file_a_directory = is_directory(emulator, file_to_remove_index);
    char* type = is_file_a_directory ? "d" : "f";

    // Remove file entry from directory data fs/<current index>
    char* parent_directory_file_path = get_directory_data_path(emulator, emulator->current_directory_inode_index);
    char* directory_data_line = to_directory_data_format(file_to_remove_index, file_name);
    remove_line_x_from_file(parent_directory_file_path, directory_data_line);

    // Remove the fs/<index> directory
    char* file_to_remove_data_directory = get_directory_data_path(emulator, file_to_remove_index);
    file_to_remove_data_directory[strcspn(file_to_remove_data_directory, "\n")] = 0; // remove trailing newline character
    remove(file_to_remove_data_directory);

    free(parent_directory_file_path);
    free(directory_data_line);
    free(file_to_remove_data_directory);

    // Remove file entry from inodes_list
    char* inodes_list_file_name = get_inodes_list_path(emulator);
    char* inode_data_line = to_inode_data_format(file_to_remove_index, type);
    remove_line_x_from_file(inodes_list_file_name, inode_data_line);

    free(inodes_list_file_name);
    free(inode_data_line);

    struct inode* node = array_list_get_item(emulator->inodes_list, string_to_int(file_to_remove_index));
    free(node->type);
    node->type = strdup("N");
    free(file_to_remove_index);
}

void invalid_syntax(char* root_command_name) {
    if (strcmp(root_command_name, "cd") == 0) {
        printf("usage: cd directory\n");
    } else if (strcmp(root_command_name, "mkdir") == 0) {
        printf("usage: mkdir directory\n");
    } else if (strcmp(root_command_name, "touch") == 0) {
        printf("usage: touch file\n");
    } else if (strcmp(root_command_name, "rm") == 0) {
        printf("usage: rm file\n");
    } else if (strcmp(root_command_name, "exit") == 0) {
        return;
    } else {
        printf("command not found: %s\n", root_command_name);
    }
}

int check_command(struct arraylist* command_words, char* root_command_name, int intended_number_of_arguments) {
    if (strcmp(root_command_name, (char*) array_list_get_item(command_words, 0)) == 0) {
        if ((command_words->number_of_items - 1) == intended_number_of_arguments) {
            return 1;
        }
    }
    return 0;
}

int verify_inode_0(struct emulator_info* emulator) {
    // check for fs/0 directory
    char* inode_0_data_directory_path = get_directory_data_path(emulator, "0");
    FILE* inode_data_directory = fopen(inode_0_data_directory_path, "r");
    if (inode_data_directory == NULL) {
        free(inode_0_data_directory_path);
        fclose(inode_data_directory);
        return 0;
    }

    // check for index 0 in inodes_list
    struct arraylist* currently_used_indexes = get_all_currently_used_inode_indexes(emulator);
    for (int i = 0; i < currently_used_indexes->number_of_items; i++) {
        char* index = array_list_get_item(currently_used_indexes, i);
        if (strcmp(index, "0") == 0) {
            array_list_cleanup(currently_used_indexes);
            free(inode_0_data_directory_path);
            fclose(inode_data_directory);
            return 1;
        }
    }

    array_list_cleanup(currently_used_indexes);
    free(inode_0_data_directory_path);
    fclose(inode_data_directory);
    return 0;
}

void emulate_shell(struct emulator_info* emulator) {
    if (!verify_inode_0(emulator)) { // an issue with inode 0
        printf("inode 0 not found or properly created, exiting\n");
        return;
    }

    char command_sentence[100] = ""; // a "command sentence" is "cd virtual_file_system" for example

    while (strcmp(command_sentence, "exit") != 0) {
        printf("cpmustang21@unix1:~ $ ");

        if (fgets(command_sentence, 100, stdin) == 0) { // EOF ; ^D ; exit
            break;
        }

        command_sentence[strcspn(command_sentence, "\n")] = 0;
        struct arraylist* command_words = split(command_sentence, " ");

        if (check_command(command_words, "cd", 1)) {
            change_directory(emulator, (char*) array_list_get_item(command_words, 1));
        } else if (check_command(command_words, "ls", 0)) {
            list_directory(emulator);
        } else if (check_command(command_words, "mkdir", 1)) {
            create_directory(emulator, (char*) array_list_get_item(command_words, 1));
        } else if (check_command(command_words, "touch", 1)) {
            touch_file(emulator, (char *) array_list_get_item(command_words, 1));
        } else if (check_command(command_words, "rm", 1)) {
            remove_file_or_directory(emulator, (char *) array_list_get_item(command_words, 1));
        } else {
            if (command_words->number_of_items >= 1) {
                invalid_syntax(array_list_get_item(command_words, 0));
            }
        }
        array_list_cleanup(command_words);
    }
}

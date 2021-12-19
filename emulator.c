#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "util/toolkit.h"
#include "util/array_list.h"
#include "emulator.h"

// TODO: pretty sure you need to add more checks about whether something is a directory or a file

struct emulator* emulator_object_create() {
//    struct emulator* emulator = malloc(sizeof(struct emulator*));
    struct emulator* emulator;
    emulator = malloc(sizeof(struct emulator*));
    emulator->inodes_list = malloc(sizeof(struct arraylist)); // ABSOLUTELY NECESSARY
    emulator->file_system_directory = NULL;
    emulator->total_supported_nodes = 0;
    emulator->current_directory_inode_index = strdup("0");
    return emulator;
}

// TODO: make some cleanup function free this memory
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

struct arraylist* get_directory_contents(struct emulator* emulator, char* inode_index) {
    // do some sort of checking to make sure the index passed is for a directory?

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

int is_directory(struct emulator* emulator, char* index) {
    for (int i = 0; i < emulator->inodes_list->number_of_items; i++) {
        struct inode* node = array_list_get_item(emulator->inodes_list, i);
        if (strcmp(node->index, index) == 0 && strcmp(node->type, "f") == 0) {
            return 1;
        }
    }
    return 0;
}

char* file_name_to_index_from_current_directory(struct emulator* emulator, char* file_name, int only_directories) {
    struct arraylist* directory_contents = get_directory_contents(emulator, emulator->current_directory_inode_index);

    for (int i = 0; i < directory_contents->number_of_items; i++) {
        struct file_node* file_node = array_list_get_item(directory_contents, i);
        if (strcmp(file_node->file_name, file_name) == 0) {
            char* file_node_index = strdup(file_node->index);
            if (only_directories && is_directory(emulator, file_node_index)) {
                file_node_array_list_cleanup(directory_contents);
                return "-2";
            }
            file_node_array_list_cleanup(directory_contents);
            return file_node_index;
        }
    }
    file_node_array_list_cleanup(directory_contents);
    return "-1";
}

int file_exists(struct emulator* emulator, char* file_name) {
    char* if_existing_file_index = file_name_to_index_from_current_directory(emulator, file_name, 0);
    int result = strcmp(if_existing_file_index, "-1") != 0;
    free(if_existing_file_index);
    return result;
}

struct arraylist* get_all_currently_used_inode_indexes(struct emulator* emulator) {
    struct arraylist* currently_used_inode_indexes = array_list_new(sizeof(char*));
    struct arraylist* inodes_list = emulator->inodes_list;

    for (int i = 0; i < inodes_list->number_of_items; i++) {
        struct inode* node = array_list_get_item(emulator->inodes_list, i);
        array_list_add_to_end(currently_used_inode_indexes, node->index);
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

char* get_available_inode_index(struct emulator* emulator, char* type) {
    struct arraylist* inodes_list = emulator->inodes_list;
    for (int i = 0; i < inodes_list->number_of_items; i++) {
        struct inode* node = array_list_get_item(inodes_list, i);
        if (strcmp(node->type, "N") == 0) {
            node->type = type; // previously deleted inode will not be used
            return node->index;
        }
    }

    struct arraylist* currently_used_inode_indexes = get_all_currently_used_inode_indexes(emulator);
    for (int i = 0; i < emulator->total_supported_nodes + 1; i++) {
        char index_char[20] = "";
        sprintf(index_char, "%d", i);

        if (!list_contains_str(currently_used_inode_indexes, index_char)) {
            return strdup(index_char);
        }
    }
    printf("operation failed: max inode limit reached\n");
    return "-1";
}

void append_to_directory_data(struct emulator* emulator, char* index, char* file_name) {
    char directory_file_path[50] = "";
    strcat(directory_file_path, emulator->file_system_directory);
    strcat(directory_file_path, "/");
    strcat(directory_file_path, emulator->current_directory_inode_index);

    char inode_string[50] = "";
    strcat(inode_string, index);
    strcat(inode_string, " ");
    strcat(inode_string, file_name);
    strcat(inode_string, "\n");

    FILE* directory_data = fopen(directory_file_path, "a");
    fwrite(inode_string, 1, strlen(inode_string), directory_data);
    fclose(directory_data);
}

void append_inodes_list(struct emulator* emulator, char* index, char* type) {
    char inodes_list_file_name[50] = "";
    strcat(inodes_list_file_name, emulator->file_system_directory);
    strcat(inodes_list_file_name, "/");
    strcat(inodes_list_file_name, "inodes_list");

    char inode_string[50] = "";
    strcat(inode_string, index);
    strcat(inode_string, " ");
    strcat(inode_string, type);
    strcat(inode_string, "\n");

    FILE* inodes_list_file = fopen(inodes_list_file_name, "a");
    fwrite(inode_string, 1, strlen(inode_string), inodes_list_file);
    fclose(inodes_list_file);
}

void create_data_file(struct emulator* emulator, char* index, char* type, char* if_file_name) {
    char new_data_file_oath[50] = "";
    strcat(new_data_file_oath, emulator->file_system_directory);
    strcat(new_data_file_oath, "/");
    strcat(new_data_file_oath, index);

    FILE* new_data_file = fopen(new_data_file_oath, "wb");

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

int strings_match(char* potentially_trailed_newline, char* string_b) {
    struct arraylist* split_data =  split(potentially_trailed_newline, "\n");
    char* string_a = array_list_get_item(split_data, 0);

    int result = strcmp(string_a, string_b) == 0;
    array_list_cleanup(split_data);
    return result;
}

void remove_line_x_from_file(char* file_name, char* line_to_remove) {
    FILE* original_file = fopen(file_name, "r");

    char file_new_copy_path[50] = "";
    strcat(file_new_copy_path, "_temp_emu");
    strcat(file_new_copy_path, file_name);

    FILE* file_new_copy = fopen(file_new_copy_path, "wb");

    char* line_buffer = NULL;
    size_t line_buffer_size = 50;

    while (getline(&line_buffer, &line_buffer_size, original_file) > 0) {
        if (strlen(line_buffer) == 1) { // ignore blank lines
            continue;
        }

        if (strings_match(line_buffer, line_to_remove)) {
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

void change_directory(struct emulator* emulator, char* to_directory_name) {
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

void list_directory(struct emulator* emulator) {
    // TODO: this is not accurate for listing the CORREct names of files (direcotires works fine)
    struct arraylist* current_directory_contents =
            get_directory_contents(emulator, emulator->current_directory_inode_index);
    for (int i = 0; i < current_directory_contents->number_of_items; i++) {
        struct file_node* file_node = array_list_get_item(current_directory_contents, i);
        printf("%s %s\n", file_node->index, file_node->file_name);
    }
    file_node_array_list_cleanup(current_directory_contents);
}

void create_directory(struct emulator* emulator, char* file_name) {
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
}

void touch_file(struct emulator* emulator, char* file_name) {
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
}

void remove_directory(struct emulator* emulator, char* file_name) {
    if (!file_exists(emulator, file_name)) {
        printf("rm: %s: No such file or director", file_name);
        return;
    }
    // remove the file

}

void invalid_syntax(char* root_command_name) {
//    if (strcmp(root_command_name, "cd") == 0) {
//
//    } else if (strcmp(root_command_name, "mkdir") == 0) {
//
//    } else if (strcmp(root_command_name, "touch") == 0) {
//
//    } else if (strcmp(root_command_name, "rm") == 0) {
//
//    }
    printf("Invalid syntax\n");
}

int check_command(struct arraylist* command_words, char* root_command_name, int intended_number_of_arguments) {
    if (strcmp(root_command_name, (char*) array_list_get_item(command_words, 0)) == 0) {
        if ((command_words->number_of_items - 1) == intended_number_of_arguments) {
            return 1;
        } else {
            invalid_syntax(root_command_name);
        }
    }
    return 0;
}

void emulate_shell(struct emulator* emulator) {

//    remove_line_x_from_file("test.txt", "3 YEAHBOI");

    char command_sentence[100] = ""; // a "command sentence" is "cd virtual_file_system" for example

    // TODO: make sure when it begins at index 0 it works properly (see Canvas Req 4)

    while (strcmp(command_sentence, "exit") != 0) { // TODO: also support EOF / CtrlD or smth like that
        printf("cpmustang21@unix1:~ $ ");
//        fflush(stdout); // causes things not to work on the linux servers for some reason

        fgets(command_sentence, 100, stdin);
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
            remove_directory(emulator, (char*) array_list_get_item(command_words, 1));
        } else {
//            invalid_syntax();
        }
        array_list_cleanup(command_words);
    }
}

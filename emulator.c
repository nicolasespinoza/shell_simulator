#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "util/toolkit.h"
#include "virtual_file_system.h"

void invalid_syntax() {
    printf("Invalid syntax. Available commands:\n");
    printf("\texit - Exit the program\n");
    printf("\tcd <directory name> - Change to a specified directory\n");
    printf("\tls - List the contents of the current working directory\n");
    printf("\tmkdir <directory name> - Create a new directory with the specified name\n");
    printf("\ttouch <file name> - Create a new empty file with the specified name\n");
}

int check_command(char* root_command_name, int intended_number_of_arguments, char** command_words) {
    int number_of_arguments = 0;
    for (int i = 0; i < 10; i++) {
        if (command_words[i] == NULL) {
            break;
        }
        number_of_arguments++;
    }
    number_of_arguments -= 1; // not counting the "root command" ie "cd"
    if (strcmp(command_words[0], root_command_name) == 0) {
        if (number_of_arguments == intended_number_of_arguments) {
            return 1;
        } else {
            invalid_syntax();
        }
    }
    return 0;
}

void emulate_shell(struct virtual_file_system* virtualFileSystem) {
    char* command_sentence = "stub"; // a "command sentence" is "cd virtual_file_system" for example

    while (strcmp(command_sentence, "exit") != 0) {
        printf("emulator $ ");

        command_sentence = get_line();
        char** command_words = str_split(command_sentence, ' ');

        if (check_command("cd", 1, command_words)) {
            change_directory(virtualFileSystem, command_words[1]);
        } else if (check_command("ls", 0, command_words)) {
            list_directory(virtualFileSystem);
        } else if (check_command("mkdir", 1, command_words)) {
            make_directory(virtualFileSystem, command_words[1]);
        } else if (check_command("touch", 1, command_words)) {
            touch_file(virtualFileSystem, command_words[1]);
        } else {
//            invalid_syntax();
        }
    }
}

void configure_virtual_file_system(struct virtual_file_system* virtualFileSystem, int number_of_arguments,
                                   char* arguments[]) {
    if (number_of_arguments == 3) {
        int total_supported_nodes = atoi(arguments[1]);
        char* starting_directory_name = arguments[2];

        virtualFileSystem->current_directory = starting_directory_name;
        virtualFileSystem->total_supported_nodes = total_supported_nodes;

        parse_files_from_directory(virtualFileSystem, starting_directory_name);
    } else {
        printf("Incorrect syntax.\nFormat: fs_simulator <number of inodes> <starting directory name>\n");
        exit(1);
    }
}

#include <stdio.h>
#include "emulator.c"

int main(int argc, char* argv[]) {
    struct virtual_file_system* virtualFileSystem;
    virtualFileSystem = create_virtual_file_system();

    configure_virtual_file_system(virtualFileSystem, argc, argv);
    emulate_shell(virtualFileSystem);


//    char* command_sentence = get_line();
//    char** command_words = str_split(command_sentence, ' ');
//
//    int result = check_command("cd", 1, command_words);
//    printf("Result: %d", result);

    printf("Program completed execution\n");
    return 0;
}

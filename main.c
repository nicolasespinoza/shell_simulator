#include <stdio.h>
#include "emulator.c"

int main(int argc, char* argv[]) {
    struct virtual_file_system* virtualFileSystem;
    virtualFileSystem = create_virtual_file_system();

    configure_virtual_file_system(virtualFileSystem, argc, argv);
    emulate_shell(virtualFileSystem);

    printf("Program completed execution");
    return 0;
}

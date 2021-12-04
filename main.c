#include "virtual_file_system.h"
#include "emulator.h"

int main(int number_of_arguments, char* arguments[]) {
    struct virtual_file_system* virtualFileSystem = create_virtual_file_system();
    configure_virtual_file_system(virtualFileSystem, number_of_arguments, arguments);
    emulate_shell(virtualFileSystem);
}

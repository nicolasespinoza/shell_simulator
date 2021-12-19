struct emulator {
    char* current_directory_inode_index;
    char* file_system_directory;
    int total_supported_nodes;
    struct arraylist* inodes_list;
};

struct emulator* emulator_object_create();
void emulate_shell(struct emulator* emulator);
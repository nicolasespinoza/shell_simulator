struct emulator_info {
    char* current_directory_inode_index;
    char* file_system_directory;
    int total_supported_nodes;
    struct arraylist* inodes_list;
};

struct emulator_info* emulator_object_create(struct arraylist* inodes_list, char* file_system_directory, int total_supported_nodes);
void emulate_shell(struct emulator_info* emulator);
struct inode* new_inode(char* index, char* type);

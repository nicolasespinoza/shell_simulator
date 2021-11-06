struct virtual_file_system {
    char* current_directory;
    int total_supported_nodes;
};

struct virtual_file_system* create_virtual_file_system();

void change_directory(struct virtual_file_system*, char*);
void list_directory(struct virtual_file_system*);
void make_directory(struct virtual_file_system*, char*);
void touch_file(struct virtual_file_system*, char*);


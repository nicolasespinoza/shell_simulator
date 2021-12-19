#include <stddef.h>

struct arraylist {
    void** data;
    size_t data_type_size;
    int number_of_items;
    int item_capacity;
};

struct inode {
    char* index; // inode value; used char* to be a lot easier
    char* type; // "d" or "f"
};

struct file_node {
    char* index;
    char* file_name;
};

struct arraylist* array_list_new(size_t data_type_size);
void array_list_add_to_end(struct arraylist* list, void* item);
void* array_list_get_item(struct arraylist* list, int index);
void array_list_cleanup(struct arraylist* list);

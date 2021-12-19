#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "toolkit.h"
#include "array_list.h"

struct arraylist* split(char* string, char* delimiter) {
    struct arraylist* split_words = array_list_new(sizeof(char*));

    char* string_copy = strdup(string); // duplicates string dynamically
    char* start = string_copy;
    char* found = NULL;
    while ((found = strsep(&string_copy, delimiter)) != NULL) {
        array_list_add_to_end(split_words, strdup(found)); // NOTE: found uses strdup here
    }
    free(start);
    free(found);

    return split_words;
}

int string_to_int(char* value) {
    char* value_copy = strdup(value);

    if (strcmp(value, "0") == 0) {
        free(value_copy);
        return 0;
    }

    char* filler;
    long converted = strtol(value_copy, &filler, 10);

    if (converted == 0) {
        printf("[%s] is not a valid integer, exiting", value);
        free(value_copy);
        exit(-1);
    }

    free(value_copy);
    return converted;
}

char* validate_file_exists(char* file_name) {
    FILE* file = fopen(file_name, "r");
    if (file != NULL) {
        fclose(file);
        return file_name;
    } else {
        printf("[%s] file could not be found, exiting\n", file_name);
        exit(-1);
    }
}

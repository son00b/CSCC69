
int count_item_in_path(char* path) {
    char *copy = malloc((strlen(path) + 1) * sizeof(char));
    if (copy == NULL) {
        perror("malloc");
        exit(1);
    }
    strcpy(copy, path);

    int count = 0;
    char *cur = strtok(copy, "/");
    while (cur != NULL)
    {
        count++;
        cur = strtok(NULL, "/");
    }
    free(copy);
    return count;
}

char *get_parent_path(int count, char* path) {
    char *copy = malloc((strlen(path) + 1) * sizeof(char));
    if (copy == NULL) {
        perror("malloc");
        exit(1);
    }
    if (count == 1) {
        strcpy(copy, "/");
    } else {
        strcpy(copy, path);
        // trim / at the end of string if it exists
        if (strlen(copy) >= 1) {
            if (copy[strlen(copy) - 1] == '/') {
                copy[strlen(copy) - 1] = 0;
            }
        }
        // remove anything after the / before the base filename
        char *final_slash = strrchr(copy, '/');
        if (final_slash) {
            *(final_slash) = 0;
        }
    }
    return copy;
}


char** arr_names(int count, char* path) {
    char *copy = malloc((strlen(path) + 1) * sizeof(char));
    if (copy == NULL) {
        perror("malloc");
        exit(1);
    }
    strcpy(copy, path);

    char **names = malloc(count *  sizeof(char*));
    int j = 0;
    char *token = strtok(copy, "/");
    while (token != NULL) {
        names[j] = malloc(EXT2_NAME_LEN * sizeof(char));
        strcpy(names[j], token);
        j++;
        token = strtok(NULL, "/");
    }
    free(copy);
    return names;
}

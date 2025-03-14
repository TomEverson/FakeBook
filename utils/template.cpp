#include "utils.h"
#include <iostream>
#include <string>
#include <unistd.h>

char *read_file(const char *input_file)
{
    char base_folder[1024];
    if (!getcwd(base_folder, sizeof(base_folder)))
    {
        std::cerr << "Error: Can't get the current working directory" << std::endl;
        return NULL;
    }

    char full_path[2048];
    snprintf(full_path, sizeof(full_path), "%s/%s", base_folder, input_file);

    FILE *file = fopen(full_path, "r");
    if (!file)
    {
        std::cerr << "Error: Cannot open template file" << std::endl;
        return NULL;
    }

    static char file_template[4096];
    size_t bytesRead = fread(file_template, sizeof(char), sizeof(file_template) - 1, file);
    file_template[bytesRead] = '\0';
    fclose(file);

    return file_template;
}

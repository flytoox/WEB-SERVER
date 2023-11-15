#include <dirent.h>
#include <iostream>
#include <sys/types.h>
#include <string>


int main(void) {


    DIR *dir_ptr; //* pointer to directory

    struct dirent *read_dir; //* structure holds the returned value of readdir function
    dir_ptr = opendir(".");

    if (dir_ptr == NULL) {
        std::cout << "Error: cannot open the file" << std::endl;
        return (1);
    }


    while ((read_dir = readdir(dir_ptr)) != NULL) {
        //* check for regular files
        if (read_dir->d_type == DT_REG) {
            std::cout << "Files: " << read_dir->d_name << std::endl;
        } else if (read_dir->d_type == DT_DIR) {
            std::cout << "Directories: " << read_dir->d_name << std::endl;
        }
    }

    if (closedir(dir_ptr) == -1) {
        std::cout << "Error: cannot close the directory" << std::endl;
    }

    return (0);
}
#include <fstream>
#include <iostream>
#include <sys/stat.h>

// copy in binary mode

// gcc 1.cpp -lstdc++ -o 1
// ./1 /Users/pavlov/Desktop/file.pdf /Users/pavlov/Desktop/newdir

bool copyFile(const char* SRC, const char* DEST)
{
    std::ifstream src(SRC, std::ios::binary);
    std::ofstream dest(DEST, std::ios::binary);
    dest << src.rdbuf();
    return src && dest;
}

char* getFileName(char* SRC, char* lastWordPtr)
{
    const char* SEPARATOR = "/";
    char *ptr = NULL;
    ptr = std::strtok(SRC, SEPARATOR);
    while (ptr){
        lastWordPtr = ptr;
        ptr = std::strtok(NULL, SEPARATOR);
    }
    return lastWordPtr;
}

int main(int argc, char* argv[])
{
//  создаем папку
    if (mkdir(argv[2], S_IRWXU))
        std::cout << "Не удалось создать папку!" << std::endl;
    
    struct stat buff;
    if (stat(argv[1], &buff))
        std::cout << "Файла не существует!" << std::endl;
    
//  получаем имя нужного файла
    char *name = NULL;
    char temp_S[256];
    strncpy(temp_S, argv[1], strlen(argv[1])+1);
    name = getFileName(temp_S, name);
    
//  задаем новое имя файла (новый путь)
    std::string newFileName(argv[2]);
    newFileName.append("/");
    newFileName.append(name);
    
//  копируем файл
    if (copyFile(argv[1], newFileName.c_str()))
        std::cout << "Файл скопирован!" << std::endl << "Путь до скопированного файла: " << newFileName << std::endl;

    return 0;
}

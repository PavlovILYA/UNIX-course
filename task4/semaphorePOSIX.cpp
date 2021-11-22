#include <semaphore.h>      /* sem_open(), sem_destroy(), sem_wait().. */
#include <fcntl.h>          /* O_CREAT, O_EXEC          */
#include <iostream>
#include <limits.h>


int main(int argc, char *argv[])
{
    char name[_POSIX_NAME_MAX];
    int value;
    char ch;
    sem_t *sem;
    
    std::cout << "Вы хотите создать новый семафор? [y (новый) / n (уже существующий)] -> ";
    std::cin >> ch;
    
    switch (ch)
    {
        case 'y':
            std::cout << "Введите название семафора (начните со знака '/') -> ";
            std::cin >> name;
            std::cout << "Задайте значение семафора -> ";
            std::cin >> value;
            sem = sem_open(name, O_CREAT | O_EXCL, 0666, value);
            if (sem != SEM_FAILED)
                std::cout << "Создали" << std::endl;
            else
            {
                std::cout << "Не удалось создать" << std::endl;
                return 0;
            }
            break;
            
        case 'n':
            std::cout << "Введите название семафора (начните со знака '/') -> ";
            std::cin >> name;
            sem = sem_open(name, 0);
            if (sem != SEM_FAILED)
                std::cout << "Подключились" << std::endl;
            else
            {
                std::cout << "Не удалось подключиться" << std::endl;
                return 0;
            }
            break;
            
        default:
            std::cout << "Введено неверное значение" << std::endl;
            return 0;
    }

    bool keepGoing = true;
    while (keepGoing)
    {
        std::cout << std::endl << "Введите:" << std::endl << "'s' - показать значение" << std::endl << "'d' - удалить семафор" << std::endl << "'w' - уменьшить значение (ожидать с блокировкой)" << std::endl << "'t' - попробовать уменьшить" << std::endl << "'f' - попробовать уменьшить (ждать определенное время)" << std::endl << "'p' - увеличить значение" << std::endl << "'e' - выход (семафор будет просто отсоединен от процесса, но НЕ удален!)" << std::endl << " -> ";
        std::cin >> ch;
        
        switch (ch)
        {
            case 's':
                if (!sem_getvalue(sem, &value))
                    std::cout << value << std::endl;
                break;
                
            case 'd':
                if(!sem_unlink(name))
                {
                    std::cout << "Удалили" << std::endl;
                    keepGoing = false;
                }
                break;
            
            case 'w':
                sem_getvalue(sem, &value);
                if (value == 0)
                    std::cout << "Сейчас значение семафора = 0, процесс будет блокирован, пока не появится возможность уменьшить значение" << std::endl;
                if (!sem_wait(sem))
                    std::cout << "Уменьшили" << std::endl;
                break;
                
            case 't':
                if (!sem_trywait(sem))
                    std::cout << "Уменьшили" << std::endl;
                else
                    std::cout << "Не получилось уменьшить" << std::endl;
                break;
                
            case 'f':
                struct timespec time;
                std::cout << "Введите количество секунд ожидания: " << std::endl;
                std::cin >> time.tv_sec;
                if (!sem_timedwait(sem, &time))
                    std::cout << "Уменьшили" << std::endl;
                else
                    std::cout << "Время истекло, не получилось уменьшить" << std::endl;
                break;
                
            case 'p':
                if (!sem_post(sem))
                    std::cout << "Увеличили" << std::endl;
                else
                    std::cout << "Не получилось увеличить" << std::endl;
                break;
                
            case 'e':
                if (!sem_close(sem))
                    std::cout << "Отсоединили семафор" << std::endl;
                keepGoing = false;
                break;
                
            default:
                std::cout << "Введено неверное значение" << std::endl;
        }
    }
    
    std::cout << "Пока!" << std::endl;
    
    return 0;
}

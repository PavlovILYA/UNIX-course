#include <stdio.h>
#include <string.h>
#include "shdata.h"

void sys_err (char *msg)
{
    std::cout << msg << std::endl;
    exit (1);
}

int main ()
{
    int semid;                                        /* идентификатор семафора */
    int shmid_f, shmid_s, shmid_t;                    /* идентификатор разделяемой памяти */
    data_t *data_p_f, *data_p_s, *data_p_t;           /* адрес данных в разделяемой памяти */
    char s[MAX_STRING];

    /* получение доступа к массиву из ТРЕХ семафоров */
    if ((semid = semget (SEM_ID, 3, 0)) < 0)
        sys_err ("client: can not get semaphore");
    std::cout << "Удалось подключиться к семафору" << std::endl;

    
    /* получение доступа к сегменту разделяемой памяти */
    if ((shmid_f = shmget (SHM_ID_F, sizeof (data_t), 0)) < 0)
        sys_err ("client: can not get shared memory segment 1");
    std::cout << "Удалось подключиться к сегменту разделяемой памяти 1" << std::endl;
    
    if ((shmid_s = shmget (SHM_ID_S, sizeof (data_t), 0)) < 0)
        sys_err ("client: can not get shared memory segment 2");
    std::cout << "Удалось подключиться к сегменту разделяемой памяти 2" << std::endl;
    
    if ((shmid_t = shmget (SHM_ID_T, sizeof (data_t), 0)) < 0)
        sys_err ("client: can not get shared memory segment 3");
    std::cout << "Удалось подключиться к сегменту разделяемой памяти 3" << std::endl;
    

    /* получение адреса сегмента */
    if ((data_p_f = (data_t *) shmat (shmid_f, 0, 0)) == NULL)
        sys_err ("server: shared memory attach error");
    std::cout << "Удалось получить адрес 1 сегмента разделяемой памяти" << std::endl;

    if ((data_p_s = (data_t *) shmat (shmid_s, 0, 0)) == NULL)
        sys_err ("server: shared memory attach error");
    std::cout << "Удалось получить адрес 2 сегмента разделяемой памяти" << std::endl;
    
    if ((data_p_t = (data_t *) shmat (shmid_t, 0, 0)) == NULL)
        sys_err ("server: shared memory attach error");
    std::cout << "Удалось получить адрес 3 сегмента разделяемой памяти" << std::endl;
    
    
    int num = 0;
    bool bKeepGoing = true;
    
    char ex[120] = "exit";
    char re[120] = "read";
    
    while (bKeepGoing)
    {
        if (num==0)
        {
            std::cout << "Введите номер сегмента разделяемой памяти (1-3) -> ";
            std::cin >> num;
            std::cout << std::endl << "Введите read, чтобы прочитать данные" << std::endl << "Введите exit, чтобы выйти" << std::endl << "Либо введите новые данные для перезаписи!" << std::endl;
            std::cout << " -> ";
            scanf ("%s", s);
        }

        while (semctl (semid, 0, GETVAL, 0) || data_p_f->type != DATA_TYPE_SYNCHRONIZED || semctl (semid, 1, GETVAL, 0) || data_p_s->type != DATA_TYPE_SYNCHRONIZED || semctl (semid, 2, GETVAL, 0) || data_p_t->type != DATA_TYPE_SYNCHRONIZED)
            /*
             *   если данные не синхронизированы или сегмент блокирован - ждать
             *                                                             */;
        
        
        semctl (semid, 0, SETVAL, 1);
        semctl (semid, 1, SETVAL, 1);
        semctl (semid, 2, SETVAL, 1);     /* блокировать */
        
        switch (num)
        {
            case 1:
                if (strcmp(s,ex)!=0)
                {
                    if (strcmp(s,re)==0)
                        std::cout << "Данные в 1 сегменте разделяемой памяти: " << data_p_f->string << std::endl;
                    else
                    {
                        /* записываем данные */
                        data_p_f->type = DATA_TYPE_CHANGED;
                        strncpy (data_p_f->string, s, MAX_STRING);
                    }
                }
                else
                {
                    /* ставим тип "завершение работы" */
                    data_p_f->type = DATA_TYPE_FINISH;
                    bKeepGoing = false;
                };
                num = 0;
                break;
            case 2:
                if (strcmp(s,ex)!=0)
                {
                    if (strcmp(s,re)==0)
                        std::cout << "Данные во 2 сегменте разделяемой памяти: " << data_p_s->string << std::endl;
                    else
                    {
                        data_p_s->type = DATA_TYPE_CHANGED;
                        strncpy (data_p_s->string, s, MAX_STRING);
                    }
                }
                else
                {
                    data_p_s->type = DATA_TYPE_FINISH;
                    bKeepGoing = false;
                };
                num = 0;
                break;
            case 3:
                if (strcmp(s,ex)!=0)
                {
                    if (strcmp(s,re)==0)
                        std::cout << "Данные в 3 сегменте разделяемой памяти: " << data_p_t->string << std::endl;
                    else
                    {
                        data_p_t->type = DATA_TYPE_CHANGED;
                        strncpy (data_p_t->string, s, MAX_STRING);
                    }
                }
                else
                {
                    data_p_t->type = DATA_TYPE_FINISH;
                    bKeepGoing = false;
                };
                num = 0;
                break;
            default:
                std::cout << "НЕВЕРНЫЙ номер сегмента разделяемой памяти" << std::endl;;
                num = 0;
        }
        semctl (semid, 0, SETVAL, 0);     /* отменить блокировку */
        semctl (semid, 1, SETVAL, 0);
        semctl (semid, 2, SETVAL, 0);
    }
    shmdt (data_p_f);                /* отсоединить сегмент разделяемой памяти */
    shmdt (data_p_s);
    shmdt (data_p_t);
    std::cout << "BYE" << std::endl;
    exit (0);
}


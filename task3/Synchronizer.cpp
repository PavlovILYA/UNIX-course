
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
    int semid;                                          /* идентификатор семафора */
    int shmid_f, shmid_s, shmid_t;                      /* идентификатор разделяемой памяти */
    data_t *data_p_f, *data_p_s, *data_p_t;             /* адрес данных в разделяемой памяти */
    char s[MAX_STRING];

    
    /* создание массива семафоров из ТРЕХ элемента */
    if ((semid = semget (SEM_ID, 3, PERMS | IPC_CREAT)) < 0)
        sys_err ("server: can not create semaphore");
    std::cout << "Удалось создать семафор" << std::endl;

    
    /* создание нескольких сегментов разделяемой памяти */
    if ((shmid_f = shmget (SHM_ID_F, sizeof (data_t), PERMS | IPC_CREAT)) < 0)
        sys_err ("server: can not create shared memory segment 1");
    std::cout << "Удалось создать shared memory segment 1" << std::endl;
    
    if ((shmid_s = shmget (SHM_ID_S, sizeof (data_t), PERMS | IPC_CREAT)) < 0)
        sys_err ("server: can not create shared memory segment 2");
    std::cout << "Удалось создать shared memory segment 2" << std::endl;
    
    if ((shmid_t = shmget (SHM_ID_T, sizeof (data_t), PERMS | IPC_CREAT)) < 0)
        sys_err ("server: can not create shared memory segment 3");
    std::cout << "Удалось создать shared memory segment 3" << std::endl;

    
    /* подключение сегментов к адресному пространству процесса */
    if ((data_p_f = (data_t *) shmat (shmid_f, 0, 0)) == NULL)
        sys_err ("server: shared memory attach error");
    std::cout << "Удалось подключить 1 разделяемую память к процессу" << std::endl;

    if ((data_p_s = (data_t *) shmat (shmid_s, 0, 0)) == NULL)
        sys_err ("server: shared memory attach error");
    std::cout << "Удалось подключить 2 разделяемую память к процессу" << std::endl;
    
    if ((data_p_t = (data_t *) shmat (shmid_t, 0, 0)) == NULL)
        sys_err ("server: shared memory attach error");
    std::cout << "Удалось подключить 3 разделяемую память к процессу" << std::endl;
    
    
    semctl (semid, 0, SETVAL, 0);             /* установка семафора */
    semctl (semid, 1, SETVAL, 0);
    semctl (semid, 2, SETVAL, 0);
    data_p_f->type = DATA_TYPE_SYNCHRONIZED; // установили, что данные синхронизированы
    data_p_s->type = DATA_TYPE_SYNCHRONIZED;
    data_p_t->type = DATA_TYPE_SYNCHRONIZED;

    while (1)
    {
        if (data_p_f->type != DATA_TYPE_SYNCHRONIZED)   // ПЕРВЫЙ СЕГМЕНТ ИЗМЕНЕН
        {
            if (semctl (semid, 0, GETVAL, 0))       /* блокировка - ждать */
                continue;

            semctl (semid, 0, SETVAL, 1);           /* установить блокировку на все! */
            semctl (semid, 1, SETVAL, 1);
            semctl (semid, 2, SETVAL, 1);

            /* обработка сообщения */
            if (data_p_f->type == DATA_TYPE_CHANGED)
            {
                std::cout << "Данные изменены в 1 сегменте разделяемой памяти: " << data_p_s->string << " -> " << data_p_f->string << std::endl;
                strncpy (data_p_s->string, data_p_f->string, MAX_STRING); // синхронизируем
                strncpy (data_p_t->string, data_p_f->string, MAX_STRING);
                std::cout << "Все сегменты разделяемой памяти синхронизованы!" << std::endl;
            }
            if (data_p_f->type == DATA_TYPE_FINISH)
                break;

            data_p_f->type = DATA_TYPE_SYNCHRONIZED; /* сообщение обработано */
            semctl (semid, 0, SETVAL, 0);            /* снять блокировку */
            semctl (semid, 1, SETVAL, 0);
            semctl (semid, 2, SETVAL, 0);
        }
        
        if (data_p_s->type != DATA_TYPE_SYNCHRONIZED)   // ВТОРОЙ СЕГМЕНТ ИЗМЕНЕН
        {
            if (semctl (semid, 1, GETVAL, 0))     /* блокировка - ждать */
                continue;

            semctl (semid, 0, SETVAL, 1);         /* установить блокировку на все! */
            semctl (semid, 1, SETVAL, 1);
            semctl (semid, 2, SETVAL, 1);

            /* обработка сообщения */
            if (data_p_s->type == DATA_TYPE_CHANGED)
            {
                std::cout << "Данные изменены во 2 сегменте разделяемой памяти: " << data_p_f->string << " -> " << data_p_s->string << std::endl;
                strncpy (data_p_f->string, data_p_s->string, MAX_STRING); // синхронизируем
                strncpy (data_p_t->string, data_p_s->string, MAX_STRING);
                std::cout << "Все сегменты разделяемой памяти синхронизованы!" << std::endl;
            }
            if (data_p_s->type == DATA_TYPE_FINISH)
                break;

            data_p_s->type = DATA_TYPE_SYNCHRONIZED; /* сообщение обработано */
            semctl (semid, 0, SETVAL, 0);            /* снять блокировку */
            semctl (semid, 1, SETVAL, 0);
            semctl (semid, 2, SETVAL, 0);
        }
        
        if (data_p_t->type != DATA_TYPE_SYNCHRONIZED)    // ТРЕТИЙ СЕГМЕНТ ИЗМЕНЕН
        {
            if (semctl (semid, 2, GETVAL, 0))     /* блокировка - ждать */
                continue;

            semctl (semid, 0, SETVAL, 1);         /* установить блокировку на все! */
            semctl (semid, 1, SETVAL, 1);
            semctl (semid, 2, SETVAL, 1);

            /* обработка сообщения */
            if (data_p_t->type == DATA_TYPE_CHANGED)
            {
                std::cout << "Данные изменены в 3 сегменте разделяемой памяти: " << data_p_s->string << " -> " << data_p_t->string << std::endl;
                strncpy (data_p_s->string, data_p_t->string, MAX_STRING); // синхронизируем
                strncpy (data_p_f->string, data_p_t->string, MAX_STRING);
                std::cout << "Все сегменты разделяемой памяти синхронизованы!" << std::endl;
            }
            if (data_p_t->type == DATA_TYPE_FINISH)
                break;

            data_p_t->type = DATA_TYPE_SYNCHRONIZED; /* сообщение обработано */
            semctl (semid, 0, SETVAL, 0);            /* снять блокировку */
            semctl (semid, 1, SETVAL, 0);
            semctl (semid, 2, SETVAL, 0);
        }
    }

    /* удаление массива семафоров */
    if (semctl (semid, 2, IPC_RMID, (struct semid_ds *) 0) < 0)
        sys_err ("server: semaphore 2 remove error");
    if (semctl (semid, 1, IPC_RMID, (struct semid_ds *) 0) < 0)
        sys_err ("server: semaphore 1 remove error");
    if (semctl (semid, 0, IPC_RMID, (struct semid_ds *) 0) < 0)
        sys_err ("server: semaphore 0 remove error");

    /* удаление сегмента разделяемой памяти */
    shmdt (data_p_f);
    if (shmctl (shmid_f, IPC_RMID, (struct shmid_ds *) 0) < 0)
        sys_err ("server: 1 shared memory remove error");
    shmdt (data_p_s);
    if (shmctl (shmid_s, IPC_RMID, (struct shmid_ds *) 0) < 0)
        sys_err ("server: 2 shared memory remove error");
    shmdt (data_p_t);
    if (shmctl (shmid_t, IPC_RMID, (struct shmid_ds *) 0) < 0)
        sys_err ("server: 3 shared memory remove error");

    std::cout << "BYE" << std::endl;
    exit (0);
}

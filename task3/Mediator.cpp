#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

#include "shdata.h"

void sys_err (char *msg)
{
    std::cout << msg << std::endl;
    exit (1);
}

int main(int argc, char *argv[])
{
    // -------------------- здесь начинается часть клиента разделяемой памяти ----------------------
    
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
        
    char ex[120] = "exit";
    char re[120] = "read";
    
     // ---------------------------------------------------------------------------------------------
    
    
    
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5000);

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    listen(listenfd, 10);
    
    std::cout << "Слушаем порт 5000" << std::endl;

    while(1)
    {
        std::cout << "слушаем!" << std::endl;
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);

        std::cout << "новое подключение!" << std::endl;
        
        std::cout << "читаем!" << std::endl;
        
        char sizeStr[4];
        read(connfd, sizeStr, 4);
        char recvBuff[atoi(sizeStr)];
        read(connfd, recvBuff, atoi(sizeStr));
        
        std::cout << "прочитали: " << recvBuff << std::endl;
        
        if (atoi(sizeStr) >= 120)
        {
            std::cout << "Строка слишком большая для сегмента разделяемой памяти" << std::endl;
            break;
        }
        
        char s[atoi(sizeStr)-1];
        for (int i = 0; i < atoi(sizeStr)-1; i++)
            s[i] = recvBuff[i+1];

        
        // -------------------- здесь начинается часть клиента разделяемой памяти ----------------------
        
        while (semctl (semid, 0, GETVAL, 0) || data_p_f->type != DATA_TYPE_SYNCHRONIZED || semctl (semid, 1, GETVAL, 0) || data_p_s->type != DATA_TYPE_SYNCHRONIZED || semctl (semid, 2, GETVAL, 0) || data_p_t->type != DATA_TYPE_SYNCHRONIZED)
        /*
         *   если данные не синхронизированы или сегмент блокирован - ждать
         *                                                             */;
        
        semctl (semid, 0, SETVAL, 1);
        semctl (semid, 1, SETVAL, 1);
        semctl (semid, 2, SETVAL, 1);     /* блокировать */
        
        switch (recvBuff[0])
        {
            case '1':
                if (strcmp(s,ex)!=0)
                {
                    if (strcmp(s,re)==0)
                    {
                        //
                        int size = strlen(data_p_f->string);
                        char sendBuff2[1024];
                        if (size>999)
                            sprintf(sendBuff2, "%d", size);
                        if (size>99 && size<1000)
                            sprintf(sendBuff2, "%c%d", '0', size);
                        if (size>9 && size<100)
                            sprintf(sendBuff2, "%c%c%d", '0', '0', size);
                        if (size<10)
                            sprintf(sendBuff2, "%c%c%c%d", '0', '0', '0', size);
                        for (int i = 0; i < size; i++)
                            sendBuff2[i+4] = data_p_f->string[i];
                        //
                        
                        std::cout << "отправляем: " << sendBuff2 << std::endl;
                        write(connfd, sendBuff2, strlen(sendBuff2));

                        std::cout << "отправили!" << std::endl;
                    }
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
                };
                break;
            case '2':
                if (strcmp(s,ex)!=0)
                {
                    if (strcmp(s,re)==0)
                    {
                        //
                        int size = strlen(data_p_s->string);
                        char sendBuff2[1024];
                        if (size>999)
                            sprintf(sendBuff2, "%d", size);
                        if (size>99 && size<1000)
                            sprintf(sendBuff2, "%c%d", '0', size);
                        if (size>9 && size<100)
                            sprintf(sendBuff2, "%c%c%d", '0', '0', size);
                        if (size<10)
                            sprintf(sendBuff2, "%c%c%c%d", '0', '0', '0', size);
                        for (int i = 0; i < size; i++)
                            sendBuff2[i+4] = data_p_s->string[i];
                        //
                        
                        std::cout << "отправляем: " << sendBuff2 << std::endl;
                        write(connfd, sendBuff2, strlen(sendBuff2));

                        std::cout << "отправили!" << std::endl;
                    }
                    else
                    {
                        data_p_s->type = DATA_TYPE_CHANGED;
                        strncpy (data_p_s->string, s, MAX_STRING);
                    }
                }
                else
                {
                    data_p_s->type = DATA_TYPE_FINISH;
                };
                break;
            case '3':
                if (strcmp(s,ex)!=0)
                {
                    if (strcmp(s,re)==0)
                    {
                        //
                        int size = strlen(data_p_t->string);
                        char sendBuff2[1024];
                        if (size>999)
                            sprintf(sendBuff2, "%d", size);
                        if (size>99 && size<1000)
                            sprintf(sendBuff2, "%c%d", '0', size);
                        if (size>9 && size<100)
                            sprintf(sendBuff2, "%c%c%d", '0', '0', size);
                        if (size<10)
                            sprintf(sendBuff2, "%c%c%c%d", '0', '0', '0', size);
                        for (int i = 0; i < size; i++)
                            sendBuff2[i+4] = data_p_t->string[i];
                        //
                        
                        std::cout << "отправляем: " << sendBuff2 << std::endl;
                        write(connfd, sendBuff2, strlen(sendBuff2));

                        std::cout << "отправили!" << std::endl;
                    }
                    else
                    {
                        data_p_t->type = DATA_TYPE_CHANGED;
                        strncpy (data_p_t->string, s, MAX_STRING);
                    }
                }
                else
                {
                    data_p_t->type = DATA_TYPE_FINISH;
                };
                break;
            default:
                std::cout << "НЕВЕРНЫЙ номер сегмента разделяемой памяти" << std::endl;
        }
        semctl (semid, 0, SETVAL, 0);     /* отменить блокировку */
        semctl (semid, 1, SETVAL, 0);
        semctl (semid, 2, SETVAL, 0);
        
        // ---------------------------------------------------------------------------------------------
        
        close(connfd);
        printf("\n Закрыли сокет \n");
        sleep(1);
     }
}

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

#include "shdata.h"


int main(int argc, char *argv[])
{
    int sockfd = 0;
    char sendBuff[1024];
    memset(sendBuff, '0', sizeof(sendBuff));
    struct sockaddr_in serv_addr;

    if(argc != 2)
    {
        printf("\n Usage: %s <ip of server> \n",argv[0]);
        return 1;
    }
    
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5000);

    if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
        return 1;
    }

    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) // соединяемся
    {
       printf("\n Error : Connect Failed \n");
       return 1;
    }
    
    std::cout << "подключились!" << std::endl;
    
    char s[120];
    std::cout << "Получилось подключиться к промежуточному серверу!" << std::endl << "Введите номер сегмента разделяемой памяти (1-3) -> ";
    std::cin >> sendBuff[0];
    std::cout << std::endl << "Введите read, чтобы прочитать данные" << std::endl << "Введите exit, чтобы выйти" << std::endl << "Либо введите новые данные для перезаписи!" << std::endl;
    std::cout << " -> ";
    scanf ("%s", s);
    for (int i = 0; i < sizeof(s); i++)
        sendBuff[i+1] = s[i];

    //
    int size = strlen(sendBuff);
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
        sendBuff2[i+4] = sendBuff[i];
    //
    std::cout << "отправляем : " << sendBuff2 << std::endl;
    
    write(sockfd, sendBuff2, strlen(sendBuff2));
    
    std::cout << "отправили!" << std::endl;
    
    char re[120] = "read";
    
    if (strcmp(s,re)==0)
    {
        char sizeStr[4];
        read(sockfd, sizeStr, 4);
        char recvBuff[atoi(sizeStr)];
        read(sockfd, recvBuff, atoi(sizeStr));
        
        std::cout << "получили: " << recvBuff << std::endl;
    }

    return 0;
}

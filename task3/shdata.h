#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdio.h>

#include <stdlib.h>
#include <iostream>

#define SEM_ID    2001      /* ключ массива семафоров */

#define SHM_ID_F    2005      /* ключ разделяемой памяти */
#define SHM_ID_S    2006      /* ключ разделяемой памяти */
#define SHM_ID_T    2007      /* ключ разделяемой памяти */

#define PERMS    0666      /* права доступа */

#define DATA_TYPE_SYNCHRONIZED  0 /* синхронизованные данные */
#define DATA_TYPE_FINISH 1 /* тип сообщения о том, что пора завершать обмен */
#define DATA_TYPE_CHANGED 2 /* данные изменены */

#define MAX_STRING    120

/* структура данных, помещаемого в разделяемую память */
typedef struct
{
//    int number;
    int type;
    char string [MAX_STRING];
}   data_t;


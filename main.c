#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <pthread.h>

int counterTX2, counterTX3;
int elems;
int buffer[15];

struct
{
    pthread_mutex_t mutex;
    int val;
    int index;
    int counterRead;
} reader = { PTHREAD_MUTEX_INITIALIZER};

struct
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int val;
    int index;
    int counterWrite;
    int nrElems;
} writer = { PTHREAD_MUTEX_INITIALIZER};

void *produce(void *);
void *consume(void *);

int main(int argc, char **argv)
{
    elems = 4000;

    pthread_t tid_TX1, tid_TX2, tid_TX3;
    pthread_create(&tid_TX1, NULL, produce, NULL);
    pthread_create(&tid_TX2, NULL, consume, NULL);
    pthread_create(&tid_TX3, NULL, consume, NULL);

    pthread_join(tid_TX1, NULL);
    pthread_join(tid_TX2, NULL);
    pthread_join(tid_TX3, NULL);

    exit(0);
}

void *produce(void *arg){
    int i;
    for(i=1;i<=elems;i++){
        while(writer.nrElems == 15){
            pthread_cond_wait(&writer.cond, &writer.mutex);
        }

        pthread_mutex_lock(&writer.mutex);
        buffer[(i-1)%15] = i;
        writer.counterWrite++;
        writer.nrElems++;

        pthread_cond_signal(&writer.cond);
        pthread_mutex_unlock(&writer.mutex);
    }

}

void *consume(void *arg){

    for(;;){
        while(writer.nrElems == 0){
            pthread_cond_wait(&writer.cond, &writer.mutex);
        }

        pthread_mutex_lock(&reader.mutex);
        if(reader.counterRead >= elems){
            printf("TX2 leu %d valores, TX3 leu %d valores\n", counterTX2, counterTX3);
            exit(0);
        }

        if(pthread_self() == 2){
            counterTX2++;
        }else counterTX3++;

        reader.val = buffer[reader.index];
        writer.nrElems--;
        reader.counterRead++;
        printf("Processo: %d, Thread:TX%d, Valor: %d, Index:%d, Volta: %d\n", getpid(), pthread_self(), reader.val, reader.index, reader.val/15);
        reader.index = (reader.index+1)%15;
        pthread_mutex_unlock(&reader.mutex);
        pthread_cond_signal(&writer.cond);
    }
}

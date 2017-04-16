//
//  main.cpp
//  Producer-consumer-problem
//
//  Created by 林理露 on 10/04/2017.
//  Copyright © 2017 林理露. All rights reserved.
//

#include <iostream>
#include <cstdio>
#include <cmath>
#include <unistd.h>
#include <ctime>
#include <cstring>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)

#ifndef BUF_SIZE
#define BUF_SIZE 4
#endif

#ifndef DATA_SIZE
#define DATA_SIZE 3
#endif

#ifndef MAX_SLEEP_TIME
#define MAX_SLEEP_TIME 3000
#endif

using namespace std;

//buffer
char buffer[BUF_SIZE];
//data
char data[DATA_SIZE];

//resource semaphore
sem_t empty,full;
//mutual exclusion semaphore
static pthread_mutex_t mt;

//buffer cursor
int i = 0,j = 0;

void *producer(void *);
void *consumer(void *);
void insert_item(const char& item,int &i);
void remove_item(char& item,int &j);

int main(int argc, const char * argv[]) {
    
    //Init
    memset(buffer, '-', sizeof(buffer));
    sprintf(data, "WQY");
    pthread_t p_tids[3],c_tids[4];
    sem_init(&empty,0,BUF_SIZE);
    sem_init(&full, 0,0);
    pthread_mutex_init(&mt, NULL);
    
    //create 3 producers
    for (int i = 0; i < 3; ++i) {
        pthread_create(&p_tids[i], NULL, producer, NULL);
    }
    
    //create 4 consumers
    for (int i = 0; i < 4; ++i) {
        pthread_create(&c_tids[i], NULL, consumer, NULL);
    }
    
    // Wait for all threads to terminate
    for (int i = 0; i < 3; ++i) {
        pthread_join(p_tids[i], NULL);
    }
    
    for (int i = 0; i < 4; ++i) {
        pthread_join(c_tids[i], NULL);
    }
    
    return 0;
}

void *producer(void *)
{
    int ctr = 0;
    while(true)
    {
        
        //Decrement empty count, if it's zero ,then block
        sem_wait(&empty);
        pthread_mutex_lock(&mt);
        
        //wait for a random amount of time
        struct timespec ts;
        ts.tv_sec = random()%MAX_SLEEP_TIME / 1000;
        ts.tv_nsec = (random()%MAX_SLEEP_TIME % 1000) * 1000000;
        nanosleep(&ts, NULL);

        //put the actual data in buffer
        insert_item(data[random()%DATA_SIZE],i);
        
        //Increment full count
        sem_post(&full);
        pthread_mutex_unlock(&mt);
        
        ++ctr;
        if (ctr == 4) {
            pthread_exit(0);
        }
    }
}

void *consumer(void *)
{
    int ctr = 0;
    while(true)
    {
        //Decrement empty count, if it's zero ,then block
        sem_wait(&full);
        pthread_mutex_lock(&mt);
        
        //wait for a random amount of time
        struct timespec ts;
        ts.tv_sec = random()%MAX_SLEEP_TIME / 1000;
        ts.tv_nsec = (random()%MAX_SLEEP_TIME % 1000) * 1000000;
        nanosleep(&ts, NULL);
        
        //put the actual data in buffer
        char taken;
        remove_item(taken,j);
        
        //Increment full count
        sem_post(&empty);
        pthread_mutex_unlock(&mt);
        
        ++ctr;
        if (ctr == 3) {
            pthread_exit(0);
        }
    }
}


void insert_item(const char& item,int &i)
{
    buffer[i] = item;
    //circular buffer
    i = (i+1) % BUF_SIZE;
    
    //display info
    printf("Producer %d put item %c from the buffer, current buffer:\n",gettid(),item);
    for (int k = 0; k < BUF_SIZE; ++k) {
        printf("%c",buffer[k]);
    }
    printf("\n");
}

void remove_item(char& item,int &j)
{
    //get the item
    item = buffer[j];
    
    //clear the position
    buffer[j] = '-';
    
    j = (j+1)%BUF_SIZE;
    
    //display info
    printf("Consumer %d took item %c from the buffer, current buffer:\n",gettid(),item);
    for (int k = 0; k < BUF_SIZE; ++k) {
        printf("%c",buffer[k]);
    }
    printf("\n");
}


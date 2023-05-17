#ifndef _BUFFER_H
#define _BUFFER_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <ledger.h>

using namespace std;

class Buffer {
  private:
    int size;
    int count;

  public:
    Buffer(int N);
    ~Buffer();
    
    void add(Ledger ledger);
    Ledger remove();
    bool isEmpty();

    pthread_mutex_t buffer_lock;
    pthread_cond_t buffer_not_full;
    pthread_cond_t buffer_not_empty;
    Ledger *buffer;
};
#endif

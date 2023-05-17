#ifndef _STORE_H
#define _STORE_H

#include <stdlib.h>
#include <fstream>
#include <string>
#include <sys/wait.h>   /* for wait() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <sys/mman.h>   /* for mmap() ) */
#include <semaphore.h>  /* for sem */
#include <assert.h>		/* for assert */
#include <iostream>     /* for cout */
#include <list>
#include <array>
#include <pthread.h>

using namespace std;


struct Resource {
  unsigned int resourceID; 
  string name;
  int amount;
  pthread_mutex_t lock;
};


class Store {
  private:
    int accountID = 0;
    int balance;
    int num;
    int num_succ;
    int num_fail;
    
  public:
    Store();
    ~Store();
    
    int get_id();
    int get_balance();
    int set_balance(int bal);
    int checkOne(int workerID, int ledgerID, int resourceID);
    int check(int workerID, int ledgerID);
    int buy(int workerID, int ledgerID, int resourceID, int amount, int money);
    int sell(int workerID, int ledgerID, int resourceID, int amount, int money);
    int trade(int workerID, int ledgerID, int src_id, int dest_id, unsigned int srcAmount, unsigned int destAmount);
    
    void print_store();
    void recordMsg(char *message);
    void recordSucc(char* message);
    void recordFail(char* message);

    pthread_mutex_t store_lock;
    struct Resource *resources;
};

#endif
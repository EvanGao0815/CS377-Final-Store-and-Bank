#include <bank.h>
#include <store.h>
#include <sstream>
#include <cstring>
#include <iostream>


/**
 * @brief Construct a new Bank:: Bank object.
 * 
 * Requirements:
 *  - The function should initialize the private variables.
 *  - Create a new array[N] of type Accounts. 
 *  - Initialize each account
 * 
 * @param N 
 */
Bank::Bank(int N) {
  pthread_mutex_init(&bank_lock, NULL);
  num = N;
  num_succ = 0;
  num_fail = 0;
  accounts = new Account[N];

  for (int i = 0; i < N; i++) {
    accounts[i].accountID = i;
    accounts[i].balance = 0;
    pthread_mutex_init(&accounts[i].lock, NULL);
  }
}


/**
 * @brief Destroy the Bank:: Bank object
 * 
 * Requirements:
 *  - Make sure to destroy all locks. 
 *  - Make sure to free all memory
 * 
 */
Bank::~Bank() {
  pthread_mutex_destroy(&bank_lock);;
  for (int i = 0; i < num; i++) {
    pthread_mutex_destroy(&accounts[i].lock);
  }
  free(accounts);
}


/**
 * @brief prints account information
 */
void Bank::print_account() {
  for (int i = 0; i < num; i++) {
    pthread_mutex_lock(&accounts[i].lock);
    cout << "ID# " << accounts[i].accountID << " | " << accounts[i].balance << endl;
    pthread_mutex_unlock(&accounts[i].lock);
  }

  pthread_mutex_lock(&bank_lock);
  cout << "Success: " << num_succ << " Fails: " << num_fail << endl;
  pthread_mutex_unlock(&bank_lock);
}

/**
 * @brief helper function to increment the bank variable `num_fail` and log 
 *        message.
 * 
 * @param message
 */
void Bank::recordFail(char *message) {
  pthread_mutex_lock(&bank_lock);
  cout << message << endl;
  num_fail++;
  pthread_mutex_unlock(&bank_lock);
}

/**
 * @brief helper function to increment the bank variable `num_succ` and log 
 *        message.
 * 
 * @param message
 */
void Bank::recordSucc(char *message) {
  pthread_mutex_lock(&bank_lock);
  cout << message << endl;
  num_succ++;
  pthread_mutex_unlock(&bank_lock);
}


/**
 * @brief Adds money to an account 
 * 
 * Requirements:
 *  - Make sure to log in the following format
 *    `Worker [worker_id] completed ledger [ledger_id]: deposit [amount] into account [account]`
 * 
 * @param workerID the ID of the worker (thread)
 * @param ledgerID the ID of the ledger entry
 * @param accountID the account ID to deposit 
 * @param amount the amount deposited
 * @return int 
 */
int Bank::check(int workerID, int ledgerID, int accountID) {
  pthread_mutex_lock(&accounts[accountID].lock);
  int bal = accounts[accountID].balance;
  string msg = "Worker " + to_string(workerID) + " completed ledger " + to_string(ledgerID) + ": check account " + to_string(accountID) + " with amount " + to_string(accounts[accountID].balance);
  recordSucc(const_cast<char*> (msg.c_str()));
  pthread_mutex_unlock(&accounts[accountID].lock);
  return bal;
}

/**
 * @brief Adds money to an account 
 * 
 * Requirements:
 *  - Make sure to log in the following format
 *    `Worker [worker_id] completed ledger [ledger_id]: deposit [amount] into account [account]`
 * 
 * @param workerID the ID of the worker (thread)
 * @param ledgerID the ID of the ledger entry
 * @param accountID the account ID to deposit 
 * @param amount the amount deposited
 * @return int 
 */
int Bank::deposit(int workerID, int ledgerID, int accountID, int amount) {
  pthread_mutex_lock(&accounts[accountID].lock); 
  accounts[accountID].balance += amount;
  string msg = "Worker " + to_string(workerID) + " completed ledger " + to_string(ledgerID) + ": deposit " + to_string(amount) + " into account " + to_string(accountID);
  recordSucc(const_cast<char*> (msg.c_str()));
  pthread_mutex_unlock(&accounts[accountID].lock);
  return 0;
}

/**
 * @brief Withdraws money from an account
 * 
 * Requirements:
 *  - Make sure the account has a large enough balance. 
 *    - Case 1: withdraw amount <= balance, log success 
 *    - Case 2: log failure
 * 
 * @param workerID the ID of the worker (thread)
 * @param ledgerID the ID of the ledger entry
 * @param accountID the account ID to withdraw 
 * @param amount the amount withdrawn
 * @return int 0 on success -1 on failure
 */
int Bank::withdraw(int workerID, int ledgerID, int accountID, int amount) {
  pthread_mutex_lock(&accounts[accountID].lock);
  int res = 0;
  if (accounts[accountID].balance >= amount) {
    accounts[accountID].balance -= amount;
    string msg = "Worker " + to_string(workerID) + " completed ledger " + to_string(ledgerID) + ": withdraw " + to_string(amount) + " from account " + to_string(accountID);
    recordSucc(const_cast<char*> (msg.c_str()));
  }
  else {
    string msg = "Worker " + to_string(workerID) + " failed to complete ledger " + to_string(ledgerID) + ": withdraw " + to_string(amount) + " from account " + to_string(accountID);
    recordFail(const_cast<char*> (msg.c_str()));
    res = -1;
  }
  pthread_mutex_unlock(&accounts[accountID].lock);
  return res;
}

/**
 * @brief Transfer from one account to another
 * 
 * Requirements:
 *  - Make sure there is enough money in the FROM account
 *  - Be careful with the locking order
 *  
 * 
 * @param workerID the ID of the worker (thread)
 * @param ledgerID the ID of the ledger entry
 * @param srcID the account to transfer money out 
 * @param destID the account to receive the money
 * @param amount the amount to transfer
 * @return int 0 on success -1 on error
 */
int Bank::transfer(int workerID, int ledgerID, int srcID, int destID, unsigned int amount) {
  int res = 0;
  if (srcID == destID) {
    string msg = "Worker " + to_string(workerID) + " failed to complete ledger " + to_string(ledgerID) + ": transfer " + to_string(amount) +  " from account " + to_string(srcID) + " to account " + to_string(destID);
    recordFail(const_cast<char*> (msg.c_str()));
    res = -1;
  } else if (srcID < destID) {
    pthread_mutex_lock(&accounts[srcID].lock);
    pthread_mutex_lock(&accounts[destID].lock);
    if (accounts[srcID].balance >= amount) {
      accounts[srcID].balance -= amount;
      accounts[destID].balance += amount;
      string msg = "Worker " + to_string(workerID) + " completed ledger " + to_string(ledgerID) + ": transfer " + to_string(amount) +  " from account " + to_string(srcID) + " to account " + to_string(destID);
      recordSucc(const_cast<char*> (msg.c_str()));
      pthread_mutex_unlock(&accounts[srcID].lock);
      pthread_mutex_unlock(&accounts[destID].lock);
    }
    else {
      string msg = "Worker " + to_string(workerID) + " failed to complete ledger " + to_string(ledgerID) + ": transfer " + to_string(amount) +  " from account " + to_string(srcID) + " to account " + to_string(destID);
      recordFail(const_cast<char*> (msg.c_str()));
      pthread_mutex_unlock(&accounts[srcID].lock);
      pthread_mutex_unlock(&accounts[destID].lock);
      res = -1;
    }
  } else {
    pthread_mutex_lock(&accounts[destID].lock);
    pthread_mutex_lock(&accounts[srcID].lock);
    if (accounts[srcID].balance >= amount) {
      accounts[srcID].balance -= amount;
      accounts[destID].balance += amount;
      string msg = "Worker " + to_string(workerID) + " completed ledger " + to_string(ledgerID) + ": transfer " + to_string(amount) +  " from account " + to_string(srcID) + " to account " + to_string(destID);
      recordSucc(const_cast<char*> (msg.c_str()));
      pthread_mutex_unlock(&accounts[destID].lock);
      pthread_mutex_unlock(&accounts[srcID].lock);
    }
    else {
      string msg = "Worker " + to_string(workerID) + " failed to complete ledger " + to_string(ledgerID) + ": transfer " + to_string(amount) +  " from account " + to_string(srcID) + " to account " + to_string(destID);
      recordFail(const_cast<char*> (msg.c_str()));
      pthread_mutex_unlock(&accounts[destID].lock);
      pthread_mutex_unlock(&accounts[srcID].lock);
      res = -1;
    }
  }
  return res;
}


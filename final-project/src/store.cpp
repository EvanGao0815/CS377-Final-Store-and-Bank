#include <bank.h>
#include <store.h>
#include <sstream>
#include <cstring>
#include <iostream>

/**
 * @brief Construct a new Bank:: Bank object. Initializes variables and creates
 * an array of resources that are each initialized
 *  
 */
Store::Store() {
  pthread_mutex_init(&store_lock, NULL);
  num = 4;
  num_succ = 0;
  num_fail = 0;
  resources = new Resource[num];
  string resNames[num] = {"Wood", "Stone", "Iron", "Gold"};
  int startingAmounts[num] = {1000, 500, 100, 20};

  for (int i = 0; i < num; i++) {
    resources[i].resourceID = i;
    resources[i].name = resNames[i];
    resources[i].amount = startingAmounts[i];
    pthread_mutex_init(&resources[i].lock, NULL);
  }
}


/**
 * @brief Destroy the Bank:: Bank object by destroying all locks and freeing all memory.
 * 
 */
Store::~Store() {
  pthread_mutex_destroy(&store_lock);;
  for (int i = 0; i < num; i++) {
    pthread_mutex_destroy(&resources[i].lock);
  }
  free(resources);
}


/**
 * @brief Returns value of all resources
 * 
 */
int Store::check(int workerID, int ledgerID) {
  for (int i = 0; i < 4; i++) {
    pthread_mutex_lock(&resources[i].lock);
    cout << "Worker " + to_string(workerID) + " checked resource " + resources[i].name + " with amount: " + to_string(resources[i].amount) + "." << endl;
    pthread_mutex_unlock(&resources[i].lock);
  }
  string msg = "Worker " + to_string(workerID) + " completed ledger " + to_string(ledgerID) + ": check resource values.";
  recordSucc(const_cast<char*> (msg.c_str()));
  return 0;
}


/**
 * @brief prints store information
 */
void Store::print_store() {
  for (int i = 0; i < num; i++) {
    pthread_mutex_lock(&resources[i].lock);
    cout << "Resource " << resources[i].name << " | " << resources[i].amount << endl;
    pthread_mutex_unlock(&resources[i].lock);
  }
  pthread_mutex_lock(&store_lock);
  cout << "Successes: " << num_succ << " Fails: " << num_fail << endl;
  pthread_mutex_unlock(&store_lock);
}

/**
 * @brief returns account id for store bank account
 */
int Store::get_id() {
  return accountID;
}

/**
 * @brief returns store balance
 */
int Store::get_balance() {
  pthread_mutex_lock(&store_lock);
  int out = balance;
  pthread_mutex_unlock(&store_lock);
  return out;
}

int Store::set_balance(int bal) {
  pthread_mutex_lock(&store_lock);
  balance = bal;
  pthread_mutex_unlock(&store_lock);
  return 1;
}

/**
 * @brief helper function to increment the bank variable `num_fail` and log 
 *        message.
 * 
 * @param message
 */
void Store::recordFail(char *message) {
  pthread_mutex_lock(&store_lock);
  cout << message << endl;
  num_fail++;
  pthread_mutex_unlock(&store_lock);
}

/**
 * @brief helper function to increment the bank variable `num_succ` and log 
 *        message.
 * 
 * @param message
 */
void Store::recordSucc(char *message) {
  pthread_mutex_lock(&store_lock);
  cout << message << endl;
  num_succ++;
  pthread_mutex_unlock(&store_lock);
}

/**
 * @brief helper function to output a message when fail or success does not need to be incremented.
 * 
 * @param message
 */
void Store::recordMsg(char *message) {
  pthread_mutex_lock(&store_lock);
  cout << message << endl;
  pthread_mutex_unlock(&store_lock);
}


/**
 * @brief Returns value of one resource
 * 
 */
int Store::checkOne(int workerID, int ledgerID, int resourceID) {
  int out = 0;
  pthread_mutex_lock(&resources[resourceID].lock);
  out = resources[resourceID].amount;
  string cMsg = "Worker " + to_string(workerID) + " checked resource " + resources[resourceID].name + " with amount: " + to_string(resources[resourceID].amount) + ".";
  recordMsg(const_cast<char*> (cMsg.c_str()));
  pthread_mutex_unlock(&resources[resourceID].lock);
  string msg = "Worker " + to_string(workerID) + " completed ledger " + to_string(ledgerID) + ": check resource values.";
  recordSucc(const_cast<char*> (msg.c_str()));
  return out;
}

/**
 * @brief Buys resources for the store
 * 
 * @param workerID the ID of the worker (thread)
 * @param ledgerID the ID of the ledger entry
 * @param resourceID the resource to buy 
 * @param amount the amount of resources bought
 * @param money the amount of money used
 * @return int 0 on success -1 on failure
 */
int Store::buy(int workerID, int ledgerID, int resourceID, int amount, int money) {
  int res = 0;
  pthread_mutex_lock(&resources[resourceID].lock);
  if (balance >= money) {
    balance -= money;
    resources[resourceID].amount += amount;
    string msg = "Worker " + to_string(workerID) + " completed ledger " + to_string(ledgerID) + ": buy " + to_string(amount) +
    " of " + resources[resourceID].name + " with " + to_string(money) + " amount of money";
    recordSucc(const_cast<char*> (msg.c_str()));
  } else {
    string msg = "Worker " + to_string(workerID) + " failed to complete ledger " + to_string(ledgerID) + ": buy " + to_string(amount) +
    " of " + resources[resourceID].name + " with " + to_string(money) + " amount of money, not enough money";
    recordSucc(const_cast<char*> (msg.c_str()));
    res = -1;
  }
  pthread_mutex_unlock(&resources[resourceID].lock);
  return res;
}

/**
 * @brief Sells resource to customer
 * 
 * @param workerID the ID of the worker (thread)
 * @param ledgerID the ID of the ledger entry
 * @param resourceID the resource to sell 
 * @param amount the amount of resources sold
 * @param money the amount of money receieved
 * @return int 
 */
int Store::sell(int workerID, int ledgerID, int resourceID, int amount, int money) {
  pthread_mutex_lock(&resources[resourceID].lock);
  int res = 0;
  if (resources[resourceID].amount >= amount) {
    resources[resourceID].amount -= amount;
    pthread_mutex_lock(&store_lock);
    balance += money;
    pthread_mutex_unlock(&store_lock);
    string msg = "Worker " + to_string(workerID) + " completed ledger " + to_string(ledgerID) + ": sell " + to_string(amount) + " of " + resources[resourceID].name + " for " + to_string(money);
    recordSucc(const_cast<char*> (msg.c_str()));
  } else {
    string msg = "Worker " + to_string(workerID) + " failed to complete ledger " + to_string(ledgerID) + ": sell " + to_string(amount) +
    " of " + resources[resourceID].name + " with " + to_string(money) + " amount of money";
    recordFail(const_cast<char*> (msg.c_str()));
    res = -1;
  }
  pthread_mutex_unlock(&resources[resourceID].lock);
  return res;
}

/**
 * @brief Trade one resource to another
 * 
 * @param workerID the ID of the worker (thread)
 * @param ledgerID the ID of the ledger entry
 * @param srcID the resource to transfer out of
 * @param destID the resource to receive into
 * @param amount the amount of source resource to transfer
 * @param value the amount of destination resource to receive
 * @return int 0 on success -1 on error
 */
int Store::trade(int workerID, int ledgerID, int srcID, int destID, unsigned int srcAmount, unsigned int destAmount) {
  int res = 0;
  if (srcID == destID) {
    string msg = "Worker " + to_string(workerID) + " failed to complete ledger " + to_string(ledgerID) + ": trade " + to_string(srcAmount) +
    " of " + resources[srcID].name + " with " + to_string(destAmount) + " of " + resources[destID].name + ", source and destination resources are the same.";
    recordFail(const_cast<char*> (msg.c_str()));
    res = -1;
  } else if (srcID < destID) {
    pthread_mutex_lock(&resources[srcID].lock);
    pthread_mutex_lock(&resources[destID].lock);
    if (resources[srcID].amount >= srcAmount) {
      resources[srcID].amount -= srcAmount;
      resources[destID].amount += destAmount;
      string msg = "Worker " + to_string(workerID) + " completed ledger " + to_string(ledgerID) + ": transfer " + to_string(srcAmount)
      + " of " + resources[srcID].name  + " to " + to_string(destAmount) + " of " + resources[destID].name;
      recordSucc(const_cast<char*> (msg.c_str()));
      pthread_mutex_unlock(&resources[srcID].lock);
      pthread_mutex_unlock(&resources[destID].lock);
    }
    else {
      string msg = "Worker " + to_string(workerID) + " failed to complete ledger " + to_string(ledgerID) + ": transfer " + to_string(srcAmount)
      + " of " + resources[srcID].name  + " to " + to_string(destAmount) + " of " + resources[destID].name + ", not enough of source resource.";
      recordFail(const_cast<char*> (msg.c_str()));
      pthread_mutex_unlock(&resources[srcID].lock);
      pthread_mutex_unlock(&resources[destID].lock);
      res = -1;
    }
  } else {
    pthread_mutex_lock(&resources[destID].lock);
    pthread_mutex_lock(&resources[srcID].lock);
    if (resources[srcID].amount >= srcAmount) {
      resources[srcID].amount -= srcAmount;
      resources[destID].amount += destAmount;
      string msg = "Worker " + to_string(workerID) + " completed ledger " + to_string(ledgerID) + ": transfer " + to_string(srcAmount)
      + " of " + resources[srcID].name  + " to " + to_string(destAmount) + " of " + resources[destID].name;
      recordSucc(const_cast<char*> (msg.c_str()));
      pthread_mutex_unlock(&resources[destID].lock);
      pthread_mutex_unlock(&resources[srcID].lock);
    }
    else {
      string msg = "Worker " + to_string(workerID) + " failed to complete ledger " + to_string(ledgerID) + ": transfer " + to_string(srcAmount)
      + " of " + resources[srcID].name  + " to " + to_string(destAmount) + " of " + resources[destID].name + ", not enough of source resource.";
      recordFail(const_cast<char*> (msg.c_str()));
      pthread_mutex_unlock(&resources[destID].lock);
      pthread_mutex_unlock(&resources[srcID].lock);
      res = -1;
    }
  }
  return res;
}
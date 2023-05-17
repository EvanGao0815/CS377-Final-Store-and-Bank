#ifndef _LEDGER_H
#define _LEDGER_H

#include <bank.h>

using namespace std;

#define D 0
#define W 1
#define T 2

const int SEED_RANDOM = 377;

struct Ledger {
	int where;
	int from;
	int to;
	int amount;
	int value;
  	int mode;
	int ledgerID;
};

extern list<struct Ledger> ledger;

void InitBankStore(int num_workers, int num_readers, char *filename);
void *fileReaderThread(void *id);
void readerHelper();
void load_ledger(char *filename);
void *worker(void *arg_struct);

#endif
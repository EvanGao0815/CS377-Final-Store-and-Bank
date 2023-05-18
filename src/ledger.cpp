#include <ledger.h>
#include <bank.h>
#include <store.h>
#include <buffer.h>

using namespace std;

pthread_mutex_t ledger_lock;
pthread_mutex_t file_lock;
list<struct Ledger> ledger;
Bank *bank;
Store *store;
Buffer *buf;
int accID;
char* FILENAME;
int numLines;


/**
 * @brief creates new bank, store, and buffer objects and sets up workers for reading and executing the ledgers.
 * After setting up the read workers, it begins to execute the multithreaded read. Currently I haven't been able to
 * fully implement the multithreaded read, so it only uses a single thread as of now.
 * 
 * Requirements:
 *  - Create a new Bank object class with 10 accounts.
 *  - Load the ledger into a list
 *  - Set up the worker threads.  
 *  
 * @param num_workers 
 * @param filename 
 */
void InitBankStore(int num_workers, int num_readers, char *filename) {
	FILENAME = filename;
	bank = new Bank(10);
	store = new Store();
	buf = new Buffer(100);
	//load_ledger(*filename);
	
	pthread_mutex_init(&file_lock, NULL);
    pthread_t readers[num_readers];
	int readerID[num_readers];
	readerHelper();

    for (int i = 0; i < num_readers; ++i) {
		readerID[i] = 1;
        pthread_create(&readers[i], NULL, fileReaderThread, &readerID[i]);
    }

    for (int i = 0; i < num_readers; ++i) {
        pthread_join(readers[i], NULL);
    }
	
	pthread_t threads[num_workers];
	int threadID[num_workers];
	pthread_mutex_init(&ledger_lock, NULL);
	accID = store->get_id();

	for (int i = 0; i < num_workers; i++) {
		threadID[i] = i;
		pthread_create(&threads[i], NULL, &worker, &threadID[i]);
    }

    for (int i = 0; i < num_workers; i++) {
        pthread_join(threads[i], NULL);
    }

	bank->print_account();
	cout << "Money | " << store->get_balance() << endl;
	store->print_store();
}


/**
 * @brief Parse a ledger file and store each line into a list
 * 
 * @param filename 
 */
/*
void load_ledger(char *filename){
	ifstream infile(filename);
	int w, f, t, a, v, m, ledgerID = 0;
	while (infile >> w >> f >> t >> a >> v >> m) {
		struct Ledger l;
		l.where = w;
		l.from = f;
		l.to = t;
		l.amount = a;
		l.value = v;
		l.mode = m;
		l.ledgerID = ledgerID++;
		buf->add(l);
	}
}
*/

/**
 * @brief Finds number of lines in the file for multithreading
 * 
 * @param filename 
 */
void readerHelper() {
    FILE *infile = fopen(FILENAME, "r");
    int val;

    while (EOF != (val = getc(infile)))
        if ('\n' == val)
            numLines ++;
}

/**
 * @brief Multithreaded ledger reader
 * 
 * @param id 
 */

void* fileReaderThread(void *id) {
    ifstream infile(FILENAME);
	int w, f, t, a, v, m, ledgerID = 0;
	int i = 0;
	pthread_mutex_lock(&file_lock);
	while (i <= numLines && infile >> w >> f >> t >> a >> v >> m) {
		struct Ledger l;
		l.where = w;
		l.from = f;
		l.to = t;
		l.amount = a;
		l.value = v;
		l.mode = m;
		l.ledgerID = ledgerID++;
		buf->add(l);
		i++;
	}
	pthread_mutex_unlock(&file_lock);
}

/**
 * @brief Remove items from the list and execute the instruction depending on the "where" and "mode" values.
 * 
 * @param workerID 
 * @return void* 
 */
void* worker(void *workerID){
	pthread_mutex_lock(&ledger_lock);
	int id = *(int*)workerID;
		while (!buf->isEmpty()) {
		Ledger currLedge = buf->remove();
		pthread_mutex_unlock(&ledger_lock);
		if (currLedge.where == 0) {
			switch(currLedge.mode) {
				case 0:
					bank->deposit(id, currLedge.ledgerID, currLedge.from, currLedge.amount);
					if (currLedge.from == 0) {
						store->set_balance(bank->check(id, currLedge.ledgerID, currLedge.from));
					}
					break;
				case 1:
					bank->withdraw(id, currLedge.ledgerID, currLedge.from, currLedge.amount);
					if (currLedge.from == 0) {
						store->set_balance(bank->check(id, currLedge.ledgerID, currLedge.from));
					}
					break;
				case 2:
					bank->transfer(id, currLedge.ledgerID, currLedge.from, currLedge.to, currLedge.amount);
					break;
				case 3:
					bank->check(id, currLedge.ledgerID, currLedge.from);
					break;
				default:
					cout << "ERROR" << endl;
			}
			pthread_mutex_lock(&ledger_lock);
		}
		else if (currLedge.where == 1) {
			switch(currLedge.mode) {
				case 0:
					store->check(id, currLedge.ledgerID);
					break;
				case 1:
					if (store->buy(id, currLedge.ledgerID, currLedge.from, currLedge.amount, currLedge.value) != -1) {
						bank->withdraw(id, currLedge.ledgerID, store->get_id(), currLedge.value);
					}
					break;
				case 2:
					if (store->sell(id, currLedge.ledgerID, currLedge.from, currLedge.amount, currLedge.value) != -1) {
						bank->deposit(id, currLedge.ledgerID, store->get_id(), currLedge.value);
					}
					break;
				case 3:
					store->trade(id, currLedge.ledgerID, currLedge.from, currLedge.to, currLedge.amount, currLedge.value);
					break;
				default:
					cout << "ERROR" << endl;
			}
			pthread_mutex_lock(&ledger_lock);
		} else {
			cout << "ERROR" << endl;
		}
	}
	pthread_mutex_unlock(&ledger_lock);
	return NULL;
}

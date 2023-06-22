# CS377-Final-Store-and-Bank
This project is the final for my CS377 Operating Systems class. This is an extension of our Project 4: Producers and Consumers which implemented a bank app that takes deposits, withdrawals and transfers through a ledger and utilizes threading .
## Project 4
The specifics for the project this was based on can be found at https://umass-cs-377.github.io/docs/projects/prodcon/
## What this project does
This specific implementation and extension of the project does a variety of things.

It creates an additional producer consumer named store that holds a certain amount of resources that can be bought and sold or traded for other resources.

Implements a Bounded Buffer that allows multiple concurrent access to a shared ledger.

Implements a check function to the Bank and Store classes.

Attempts to multithread the read-in operation of the ledger.

Extends the ledger text format from "from to amount mode" to "where from to amount value mode", where "where" describes whether the transaction is for the bank or the store and value is a secondary amount for functions that require it e.g. trade.
## Installation and Usage
Download the code and copy it into your C++ environment and run
```bash
make
```
You can then run
```bash
./bank_store_app
```
to run the application.

The project is similar to the class projects in how to download and execute.

You can change the ledger values in exampleLedger.txt, with "where from to amount value mode" instead of "from to amount mode".
## Design Decisions
I wanted this project to be an extension for project 4 to include a working pretend storefront. Originally, my first design was far too ambitious and I scaled the project back. I wanted to create a version of the project more heavily involved with locks and producers and consumers and that is what I attempted to make. 

I wanted to have the store functions use the bank ones as much as possible, but this would only be possible in the ledger.cpp file since the bank instance would not be initialized. As such, for the switch case that determined which mode to use, I added the bank operations to those to update the store account balance.

I have the balance variable in the store class. I originally wanted it to be directly connected to the store's account in the bank, using things such as extern or linked objects between the classes but was not able to do so.
## Implementation Specifics
For specifics on functions already implemented, refer to the Project 4 documentation.
## Libraries
This project uses the same standard libraries as Project 4, I didn't import anything new.
### Bounded Buffer
The bounded buffer is a finite size struct that acts as an array of Ledger objects that are eventually taken by worker threads and executed. It takes an input int N, and makes the array size N.

Buffer and ~Buffer are the constructor and destructors.

add() takes the an incoming ledger object and appends it to the end of the list, which is stored in the variable count. This only occurs if the buffer is not full, and signals that the buffer is not empty.

remove() removes the ledger stored at the beginning of the list and returns it. It then shifts down every other element towards start of the list such that the next ledger in line is now in index 0. This only occurs if the buffer is not empty, and signals that the buffer is not full.

isEmpty() returns if the count is equal to the max size of the buffer size.
### Bank
Changes to Bank:
The function check() takes an accountID and returns the balance of the accounts, locking and unlocking the mutex for the accound before the operations.
### Store
Store constructor: As a part of the constructor, it creates a new resource array currently of fixes size N, and initialize each resource properly with ID, name, and amount and the resource lock.

Store destructor: This frees all memory and destroys all locks. This loops through each of the resource locks and destroys them, in addition to the store lock. Finally, it frees the resources struct of its memory.

The get and set balance functions either returns the balance variable or assigns an input int n to it.

The check function is almost the same as the print_store and print_bank functions, except it itslef can be called by the ledger, and as such records a success whenever it executes correctly. It iterates through each of the resources and prints the amount of the resource.

The print_store function does the same and then the number of successes and failures.

The buy function recreates the store buying more resources from another supplier. It takes as input the resourceID of the resource it wants to buy, the amount it wants to buy, and the amount of money that they want to use to buy it. It first checks if there is enough balance for the store to buy the resources, then subtracts that amount from the store balance and adds the appropriate amount of resource it wanted to buy. It then records as a success. If there is not enough money to buy the resources, then it records a failure.

The sell function recreates the store selling resources to a customer. It takes as input the resourceID of the resource it wants to sell, the amount it wants to buy, and the amount of money that they want to sell it for. It first checks if there is enough of the resource in the store to sell the resources, then subtracts that amount from the resources struct, and adds the appropriate amount of money. It then records as a success. If there is not enough resource to sell, then it records a failure.

The trade function recreates the store wanting to trade an amount of one resource with another at another supplier. It takes as input the srcID of the resource it has and another destID of the resource it wants to buy, and the amounts of each it wants to trade. It first checks if there is enough of the srcID resource for trade for, then subtracts that amount from the store balance and adds the appropriate amount of resource it wanted to trade for. It then records as a success. If there is not enough resource to trade away, then it records a failure.
### Ledger
Changes to Ledger:
The function InitBankStore() is the replacement for InitBank and combines initializations for both Bank and Store. This was done in order to share worker threads for both of these. It was originally planned for the two to have seperate workers, but due to time constraints I combined the two for now.

The function fileReaderThread() replaces the load_ledger method. It takes a file name as an input and parses each line of the file into struct ledger objects and appends it to the bounded buffer. Currently is not fully functional with multiple threads.
  
The function readerHelper() reads in the file in order to determine how many lines are in the ledger, which helps with knowing when to stop the ledger readers.

The function worker is similar to the worker function for the original project. However, it now checks a "where" portion of the current ledger in order to determine if the transaction is for the bank or the store. If it is for the store, whenever you buy or sell a resource, it also calls the bank deposit and withdraw in order to update the store's bank account.

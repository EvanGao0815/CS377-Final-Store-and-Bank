#include <ledger.h>

int main(int argc, char* argv[]) {
  if (argc != 4) {
    cerr << "Usage: " << argv[0] << " <num_of_threads> <num_of_reader_threads> <leader_file>\n" << endl;
    exit(-1);
  }

  int p = atoi(argv[1]);
  int r = atoi(argv[2]);
  InitBankStore(p, r, argv[3]);

  return 0;
}

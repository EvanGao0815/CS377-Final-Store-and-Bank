#include <buffer.h>

/**
 * @brief Buffer initialization
 * 
 * @param N Size of buffer
 */

Buffer::Buffer(int N) {
  buffer = new Ledger[N];
  size = N;
  count = 0;
  pthread_mutex_init(&buffer_lock, NULL);
  pthread_cond_init(&buffer_not_full, NULL);
  pthread_cond_init(&buffer_not_empty, NULL);
}


/**
 * @brief Buffer destructor
 * 
 */
Buffer::~Buffer() {
  free(buffer);
  pthread_mutex_destroy(&buffer_lock);
}

/**
 * @brief Receives ledger from threaded ledger reader and puts it in the buffer at location size - count
 * 
 * @param data Ledger to be added to the buffer
 */
void Buffer::add(Ledger data) {
  pthread_mutex_lock(&buffer_lock);
  while(count == size){
      pthread_cond_wait(&buffer_not_full, &buffer_lock);
  }
  buffer[count] = data;
  count++;
  pthread_cond_signal(&buffer_not_empty);
  pthread_mutex_unlock(&buffer_lock);
}

/**
 * @brief Removes ledger from the end of the buffer and puts it in the buffer at location size - count
 * 
 */
Ledger Buffer::remove() {
  pthread_mutex_lock(&buffer_lock);
  while(isEmpty()){
    pthread_cond_wait(&buffer_not_empty, &buffer_lock);
  }
  Ledger output = buffer[0];
  for (int i = 0; i <= count; i++) {
    buffer[i] = buffer[i + 1];
  }
  count--;
  pthread_cond_signal(&buffer_not_full);
  pthread_mutex_unlock(&buffer_lock);
  return output;
}

bool Buffer::isEmpty() {
  return count == 0;
}

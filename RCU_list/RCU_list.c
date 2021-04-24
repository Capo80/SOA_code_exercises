#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


typedef struct rcu_list
{
	int key;
	struct rcu_list* next;

} rcu_list;

// reader counter
unsigned int* reader_counter;

// writer mutex
pthread_mutex_t writer_mutex;

// return a new node
rcu_list* create_node(long key) {

	rcu_list* new_node = (rcu_list*) malloc(sizeof(rcu_list));
	new_node->key = key;
	new_node->next = NULL;

	return new_node;
}

// Insert at the front of the queue
// Allow single writer thread
int rcu_list_insert(rcu_list **l, long key) {
	rcu_list* new_node = create_node(key);
	pthread_mutex_lock(&writer_mutex);
	new_node->next = *l;
	*l = new_node;
	pthread_mutex_unlock(&writer_mutex);
	return 0;
}

// Search in queue
// Assume assume multiple thread
int rcu_list_search(rcu_list *l, long key) {

	//update reader counter
	__sync_fetch_and_add(reader_counter, 1);

	//search list
	while (l != NULL && l->key != key)
		l = l->next;

	if (l)
		return -1;
	else
		return 0;

	//update reader counter
	__sync_fetch_and_sub(reader_counter, 1);
}

// Remove item from list
// Allow single writer thread
int rcu_list_remove(rcu_list *l, long key)  {

	pthread_mutex_lock(&writer_mutex);

	//update structure
	if (l == NULL)
		return -1;

	while (l->next != NULL && l->next->key != key)
		l = l->next;

	if (l->next == NULL) 
		return -1;

	rcu_list* to_remove = l->next;
	l->next = l->next->next;

	//grace period
	unsigned int* new_reader_counter = malloc(sizeof(unsigned int));
	int* active_readers = __sync_val_compare_and_swap(&reader_counter, &reader_counter, &new_reader_counter);

	while (*active_readers > 0);

	//release buffers
	free(to_remove);
	free(active_readers);

	pthread_mutex_unlock(&writer_mutex);
}

void main() {


}
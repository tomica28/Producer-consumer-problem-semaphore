#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>


// Zmienne globalne
typedef int buffer_item;
buffer_item *buffer;
long int buff_size;
long int num_producer;
long int num_items;
pthread_mutex_t mutex;
sem_t full, empty;
int count, in, out;

int insert_item(buffer_item item);
int remove_item(buffer_item *item);
void *consumer(void *param);
void *producer(void *param);
int setId(void);

int main(int argc, char **argv){
  if (argc != 4){
    printf("ERROR: Podaj trzy argumenty wywaołania\n");
    exit(1);
  }

  // Pobranie danych od użytkownika
  num_producer = strtol(argv[1], NULL, 0);
  buff_size = strtol(argv[2], NULL, 0);
  num_items = strtol(argv[3], NULL, 0);

  // Zaalokowanie miejsca na bufor oraz inicjalizacja semaforów i zmiennych pomocniczych
  buffer = malloc(sizeof(int)*buff_size);
  int i;
  srand(time(NULL));
  pthread_mutex_init(&mutex, NULL);
  sem_init(&empty, 0, buff_size); 
  sem_init(&full, 0, 0);
  count = in = out = 0;

  // Stworzenia wątków producentów i konsumenta
  pthread_t producers[num_producer];
  pthread_t consumers;
  for(i = 0; i < num_producer; i++)
    pthread_create(&producers[i], NULL, producer, NULL);
  pthread_create(&consumers, NULL, consumer, NULL);

  // Czekanie na zakonczenie wykonywania watkow
  for(i = 0; i < num_producer; i++)
	pthread_join(producers[i], NULL);
  pthread_join(consumers, NULL);
  // Zwolnienie bufora
  free(buffer);
  return 0;
}

// Funkcja sluzaca do wkladania elementow do bufora
//Zwraca 0 w przypadku sukcesu oraz -1 w przypadku bledu
int insert_item(buffer_item item){
  int success;
  sem_wait(&empty);
  pthread_mutex_lock(&mutex);

  // Add item to buffer
  if( count != buff_size){
    buffer[in] = item;
    in = (in + 1) % buff_size;
    count++;
    success = 0;
  }
  else
    success = -1;

  pthread_mutex_unlock(&mutex);
  sem_post(&full);

  return success;
}

// Funkcja służąca do wyjmowania produktów z bufora
//Zwraca 0 w przypadku sukcesu oraz -1 w przypadku bledu
int remove_item(buffer_item *item){
  int success;

  sem_wait(&full);
  pthread_mutex_lock(&mutex);

  if( count != 0){
    *item = buffer[out];
    out = (out + 1) % buff_size;
    count--;
    success = 0;
  }
  else
    success = -1;

  pthread_mutex_unlock(&mutex);
  sem_post(&empty);

  return success;
}

void *producer(void *param){
  buffer_item item;
  int id = setId();
  int i = 0;
  while(i < num_items){
    sleep(rand() % 5 + 1); // Wykonywanie pracy losowo miedzy 1 a 5 sekund

    item = rand();
    if(insert_item(item))
      printf("Error occured\n");
    else
      printf("Producer %d produced %d number of produkt %d\n", id, item, i);
    i++;
  }
}


void *consumer(void *param){
  buffer_item item;
  int i = 0;
  while(i < num_items*num_producer){
    sleep(rand() % 5 + 1); // Wykonywanie pracy losowo miedzy 1 a 5 sekund

    if(remove_item(&item))
      printf("Error occured\n");
    else
      printf("Consumer consumed %d\n", item);
    i++;
  }
}

//Funkcja służaca do nadawania numerów id wątkom producenta
int setId(void){
	static int id = 0;
	int temp = id;
	id++;
	return temp;
}


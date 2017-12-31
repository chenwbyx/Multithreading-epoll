//queue.h  
  
#ifndef QUEUE_H_INCLUDED  
#define QUEUE_H_INCLUDED  
  
typedef struct queue *queue_t;  
  
queue_t queue_create();  
  
int queue_isempty(queue_t q);  
  
void* queue_enqueue(queue_t q, unsigned int bytes);  
  
void* queue_dequeue(queue_t q);  
  
void queue_destroy(queue_t q);  
  
#endif  //QUEUE_H_INCLUDED 
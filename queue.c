//queue.c  
  
#include "queue.h"  
#include <stdlib.h>  
  
struct node {  
    void *element;  
    struct node *next;  
};  
  
struct queue {  
    struct node front;  
    struct node *tail;  
};  
  
queue_t queue_create() {  
    queue_t q;  
    q=(queue_t)malloc(sizeof(struct queue));  
    q->front.element=NULL;  
    q->front.next=NULL;  
    q->tail=&q->front;  
    return q;  
}  
  
int queue_isempty(queue_t q) {  
    return &q->front==q->tail;  
}  
  
void* queue_enqueue(queue_t q, unsigned int bytes) {  
    q->tail->next=(struct node*)malloc(sizeof(struct node));  
    q->tail->next->element=malloc(bytes);  
    q->tail->next->next=NULL;  
    q->tail=q->tail->next;  
    return q->tail->element;  
}  
  
void* queue_dequeue(queue_t q) {  
    struct node *tmp=q->front.next;  
    void *element;  
    if(tmp==NULL) {  
        return NULL;  
    }  
    element=tmp->element;  
    q->front.next=tmp->next;  
    free(tmp);  
    if(q->front.next==NULL) {  
        q->tail=&q->front;  
    }  
    return element;  
}  
  
void queue_destroy(queue_t q) {  
    struct node *tmp, *p=q->front.next;  
    while(p!=NULL) {  
        tmp=p;  
        p=p->next;  
        free(tmp);  
    }  
    free(q); // 感谢@Toudsour指正  
}  
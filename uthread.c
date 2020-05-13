// User-space thread library


// Of course, my header file!
#include "uthread.h"

#include <stdlib.h> // malloc and other things
#include <stdio.h> // some debug printing


// Number of bytes in a Mebibyte, used for thread stack allocation
#define MEM 1048567


// The threads are managed with a double-linked list
static ucontext_t *main_ctx; // original (main) context
static int num_threads = 0; // Used to count the number of linked list nodes and give thread_ids

// Inserts a node (at the back) into the linked-list
void ll_insert(node *new_node){
	if(head == NULL){
		head = new_node;
		tail = new_node;
		//printf("head tid: %u\n", head->t->tid);
	} else {
		tail->next = new_node;
		tail = new_node;
	}
}

// print all the threads in the linked list
void ll_print(){
	int counter = 0;
	printf("-- Linked List of Threads --\n");
	node *cur = head;
	while(cur != NULL){
		printf("Node %d with tid %u and state: %d\n", counter, cur->t->tid, cur->t->state);
		cur = cur->next;
		counter++;
	}
	printf("--Linked List of Threads --\n");
}

// for a given state, return the first thread node of matching state
node *ll_get_by_state(int state){
	node *cur = head;
	while( (cur != NULL) && (cur->t->state != state) ){
		//printf("thread tid: %u state %d\n", cur->t->tid, cur->t->state);
		cur = cur->next;
	}
	return cur;
}

// for a given tid, return the thread node of matching tid
node *ll_get_by_tid(unsigned int tid){
	node *cur = head;
	while( (cur != NULL) && (cur->t->tid != tid)){
		cur = cur->next;
	}
	return cur;
}
void returnMain() // The function for the return to main thread
{
	setcontext(main_ctx);
}

// Feel free to implement utility methods as you see fit.

/** These are the methods you need to implement! **/
void uthread_init(void){
	//printf("init called\n");

	// Implement the necessary structs and state so that the first call to uthread_create is successful
	// You should assume that this method is called by the program's main / original thread.
	// Set the value of static ucontext_t *main_ctx (declared above)

	// 1) Make main context  (and place into main_ctx)
	// 2) Make uthread_t
	// 3) Make node
	// 4) Insert into linked list
	main_ctx= malloc(sizeof(ucontext_t));
	getcontext(main_ctx);
	
	uthread_t *main_thread= malloc(sizeof(uthread_t));
	main_thread->tid=100+num_threads;
	main_thread->ctx=main_ctx;
	main_thread->joining_tid=-1;
	main_thread->state=T_ACTIVE;
	num_threads = num_threads+1;
	node *main_node=malloc(sizeof(node));
	main_node->t=main_thread;
	main_node->next=NULL;
	ll_insert(main_node);
}

uthread_t *uthread_create(void *(*func)(void *), void *argp){
	//printf("create called\n");
	// Create a new thread which will call func
	// 1) Make main context  (and place into main_ctx)
	// 2) Make uthread_t
	// 3) Make node
	// 4) Insert into linked list
	// 5) Schedule this thread to run!
	// 6) Return the new uthread_t that was created
	ucontext_t *T2 = malloc(sizeof(ucontext_t));
	getcontext(T2);
	T2->uc_link=0;
	T2->uc_stack.ss_sp=malloc(MEM);
	T2->uc_stack.ss_size=MEM;
	T2->uc_stack.ss_flags=0;
	makecontext(T2, (void*)&returnMain, 0);
	
	ucontext_t *T1 = malloc(sizeof(ucontext_t));
	getcontext(T1);
	T1->uc_link=T2;
	T1->uc_stack.ss_sp=malloc(MEM);
	T1->uc_stack.ss_size=MEM;
	T1->uc_stack.ss_flags=0;
	makecontext(T1, (void *)func,1,(void *)argp);
	
	uthread_t *thread= malloc(sizeof(uthread_t));
	thread->tid=100+num_threads;
	thread->ctx=T1;
	thread->joining_tid=0;
	thread->state=T_ACTIVE;
	num_threads = num_threads+1;
	node *thread_node=malloc(sizeof(node));
	thread_node->t=thread;
	thread_node->next=NULL;
	node *oldActive=ll_get_by_state(0);
	oldActive->t->state=T_READY;
	ll_insert(thread_node);
	swapcontext(main_ctx, T1);
	oldActive->t->state=T_ACTIVE;
	thread->state=T_READY;
	return thread;
}

int uthread_get_id(void){
	//printf("get_id called\n");
	// Search through queue (linked list) 
	// for the active thread and return it's tid
	
	node *active=ll_get_by_state(0);
	int activeId= active->t->tid;
	return activeId;
}

int uthread_yield(void){
	//printf("yield called\n");
	// Just schedule some other thread!
	

	// return 0 if there are no errors
	
	node *active=ll_get_by_state(0);
	node *cur=active->next;
	if(cur==NULL){
		cur=head;
	}
	while(cur!=NULL && cur->t->state != 1){
		if (cur==tail){
			cur=head;
		}
		else if(cur==active){
			return -1;
		}
		else{
			cur=cur->next;
		}
	}
	active->t->state=T_READY;
	cur->t->state=T_ACTIVE;
	swapcontext(active->t->ctx,cur->t->ctx);
	return 0;
}

int uthread_join (uthread_t thread){
	//printf("join called\n");
	// Example, main thread calls uthread_join(child_thread_1) 
	//
	// 1) Find the given thread in the queue (linked list) by it's TID
	// 
	// 2) Tell the given thread that it is blocking the calling
	//    thread (i.e. the currently active thread), by setting 
	//    the joining_tid
	//
	// 3) Mark the calling (currently active) thread as blocked
	// 
	// 4) Mark the given thread (from the queue) as active
	// 
	// 5) swap the context to start the given thread (from the queue)
	
	// return 0 if there are no errors
	
	node *givenThread=ll_get_by_tid(thread.tid);
	node *activeThread=ll_get_by_state(0);
	givenThread->t->joining_tid=activeThread->t->tid;
	activeThread->t->state=T_BLOCKED;
	givenThread->t->state=T_ACTIVE;
	swapcontext(activeThread->t->ctx,givenThread->t->ctx);
	return 0;
	
	
	
}
/** These are the methods you need to implement! **/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

typedef struct memBlock {
    int        size;
    struct memBlock   *prev;
    struct memBlock   *next;
}        memBlock;

static memBlock *head = NULL;
void* heap = NULL;

//Remove memBlock b from the free linked list, while keeping order by address
void free_remove(memBlock* b){
    //If there is no free memory block before b
    if(!b->prev){
        //If there is a free memory block after b set head after b
        if(b->next) head = b->next;
        //Else set head to NULL
        else head = NULL;
    }
    //Else set b->next as b->prev's next node
    else b->prev->next = b->next;
    //If there is a free memory block after b, set it as the previous node for b->next
    if(b->next) b->next->prev = b->prev;
}

//Add memBlock b to the free linked list, while keeping order by address
void free_add(memBlock* b){
    b->prev = NULL;
    b->next = NULL;
    //If the address of b is before head's address and make it the new head
    if(!head || (unsigned long) head > (unsigned long)b){
        if(head) head->prev = b;
        b->next = head;
        head = b;
    }
    else{
        memBlock* itr = head;
        //Go through the linked list until you find the correct address to put b in
        while(itr->next && (unsigned long)itr->next < (unsigned long)b) itr = itr->next;
        //Put b in that spot
        b->next = itr->next;
        itr->next = b;
    }
}

//Go through the linked list and merge memory blocks with consecutive memory addresses
void merge(){
    memBlock* itr = head;
    unsigned long header_itr, header_next;
    //Iterate through the linked list
    while(itr->next){
        header_itr = (unsigned long)itr;
        header_next = (unsigned long)itr->next;
        
        //If the addresses are continuous merge them
        if(header_itr + (itr->size) + sizeof(memBlock) == header_next){
            itr->size += itr->next->size;
            itr->next = itr->next->next;
            if(itr->next) itr->next->prev = itr;
            else break;
        }
        itr = itr->next;
    }
}

//Splits the memory block b into one with the specified size and the leftovers
memBlock* split(memBlock* b, int size){
    //make new memBlock with size of memBlock b
    void* newMemBlock = ((void *)((unsigned long)b + sizeof(memBlock)));
    //make new pointer at memBlock address
    memBlock* newPtr = (memBlock*) ((unsigned long)newMemBlock + size);
    //set new pointer's size equal to the left over size of memory)
    newPtr->size = b->size - size;
    //set b size to specified size
    b->size = size;
    return newPtr;
}

//Allocates requested amount of memory, size
void* myMalloc(int size){
    void* newMemBlock;
    memBlock *itr, *newPtr;
    int allocSize = size + sizeof(memBlock);
    itr = head;
    while(itr){
        if(itr->size >= size){
            newMemBlock = ((void *)((unsigned long)itr + sizeof(memBlock)));
            free_remove(itr);
            if(itr->size == size) return newMemBlock;
            newPtr = split(itr, size);
            free_add(newPtr);
            return newMemBlock;
        }
        else itr = itr->next;
    }
    //No free block in the free list left, push heap ptr and allocate memory
    heap += allocSize;
    itr = heap;
    itr->next = NULL;
    itr->prev = NULL;
    itr->size = allocSize - sizeof(memBlock);
    return ((void *)((unsigned long)itr + sizeof(memBlock)));
}

//Removes the ptr from the linked list
void myFree(void *ptr){
    //Add ptr to the free linked list
    free_add(((void *)((unsigned long)ptr - sizeof(memBlock))));
    //Merge the free linked list
    merge();
}

//Prints out debugging information
void debugInfo(char *prefix){
    printf("[%s] \n", prefix);
    printf("Heap pointer address: %p\n", heap);
    int i = 0;
    memBlock* itr = head;
    printf("[%s] Free list: \n", prefix);
    while (itr) {
        printf("(%d) <%10p> (size: %i(%05x), total size: %i(%05x))\n", i, itr, itr->size, itr->size, itr->size + sizeof(memBlock), itr->size + sizeof(memBlock));
        itr = itr->next;
        i++;
    }
}

int main(int argc, char const *argv[]){
    heap = sbrk(0);
    printf("Size of memBlock: %i(%03x)\n", sizeof(memBlock), sizeof(memBlock));
    debugInfo("No allocation");
    char *ptr, *ptr2, *ptr3;
    ptr = (char*)myMalloc(1);
    debugInfo("Allocate ptr");
    ptr3 = (char*)myMalloc(3);
    debugInfo("Allocate ptr3");
    myFree(ptr3);
    debugInfo("Free ptr3");
    myFree(ptr);
    debugInfo("Free ptr1");
    ptr2 = (char*)myMalloc(2);
    debugInfo("Allocate ptr2");
    myFree(ptr2);
    debugInfo("Free ptr2");
    return 0;
}

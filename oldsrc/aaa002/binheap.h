


#ifndef BINHEAP_H
#define BINHEAP_H

#include "../platform.h"

class TryStep;

// http://www.sourcetricks.com/2011/06/c-heaps.html

class Heap 
{
public:
    Heap();
    ~Heap();
    bool insert(TryStep* element);
    TryStep* deletemin();
	bool hasmore();
	void alloc(int ncells);
	void freemem();
	void resetelems();
#if 0
    void print();
#endif
private:
    int left(int parent);
    int right(int parent);
    int parent(int child);
    void heapifyup(int index);
    void heapifydown(int index);
private:
#if 0
    TryStep** heap;
	int nelements;
	int allocsz;
#else
	vector<TryStep*> heap;
#endif
};

#endif
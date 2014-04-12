

#include "binheap.h"
#include "trystep.h"
#include "../utils.h"

Heap::Heap()
{
#if 0
	heap = NULL;
	allocsz = 0;
	nelements = 0;
#endif
}

Heap::~Heap()
{
	freemem();
}

void Heap::alloc(int ncells)
{
#if 0
	freemem();

	heap = new TryStep* [ ncells ];
	allocsz = ncells;
	nelements = 0;
#endif
}

void Heap::freemem()
{
#if 0
	if(heap)
	{
		delete [] heap;
		heap = NULL;
	}

	allocsz = 0;
	nelements = 0;
#else
	heap.clear();
#endif
}

void Heap::resetelems()
{
#if 0
	nelements = 0;
#else
	heap.clear();
#endif
}

bool Heap::insert(TryStep* element)
{
#if 0

	if(nelements >= allocsz)
		return false;

	heap[nelements] = element;
	nelements++;
	heapifyup(nelements - 1);
#else
	heap.push_back(element);
#endif

	return true;
}

bool Heap::hasmore()
{
#if 0
	return nelements > 0;
#else
	return heap.size() > 0;
#endif
}

TryStep* Heap::deletemin()
{
#if 0
	TryStep* min = heap[0];
	heap[0] = heap[nelements - 1];
	nelements--;
	heapifydown(0);
	return min;
#else
	TryStep* min = heap.front();
	heap[0] = heap.at(heap.size() - 1);
	heap.pop_back();
	heapifydown(0);
	return min;
#endif
}

#if 0
void Heap::print()
{
	vector<int>::iterator pos = heap.begin();
	cout << "Heap = ";
	while ( pos != heap.end() ) {
		cout << *pos << " ";
		++pos;
	}
	cout << endl;
}
#endif

void Heap::heapifyup(int index)
{    
	//cout << "index=" << index << endl;
	//cout << "parent(index)=" << parent(index) << endl;
	//cout << "heap[parent(index)]=" << heap[parent(index)] << endl;
	//cout << "heap[index]=" << heap[index] << endl;
	while ( ( index > 0 ) && ( parent(index) >= 0 ) &&
		( heap[parent(index)]->F > heap[index]->F ) )
	{
		TryStep* tmp = heap[parent(index)];
		heap[parent(index)] = heap[index];
		heap[index] = tmp;
		index = parent(index);
	}
}

void Heap::heapifydown(int index)
{     
	//cout << "index=" << index << endl;
	//cout << "left(index)=" << left(index) << endl;
	//cout << "right(index)=" << right(index) << endl;
	int child = left(index);
	if ( ( child > 0 ) && ( right(index) > 0 ) &&
		( heap[child]->F > heap[right(index)]->F ) )
	{
		child = right(index);
	}
	if ( child > 0 )
	{
		TryStep* tmp = heap[index];
		heap[index] = heap[child];
		heap[child] = tmp;
		heapifydown(child);
	}
}

int Heap::left(int parent)
{
	int i = ( parent << 1 ) + 1; // 2 * parent + 1
#if 0
	return ( i < nelements ) ? i : -1;
#else
	return ( i < heap.size() ) ? i : -1;
#endif
}

int Heap::right(int parent)
{
	int i = ( parent << 1 ) + 2; // 2 * parent + 2
#if 0
	return ( i < nelements ) ? i : -1;
#else
	return ( i < heap.size() ) ? i : -1;
#endif
}

int Heap::parent(int child)
{
	if (child != 0)
	{
		int i = (child - 1) >> 1;
		return i;
	}
	return -1;
}

#if 0
int main()
{
	// Create the heap
	Heap* myheap = new Heap();
	myheap->insert(700);
	myheap->print();
	myheap->insert(500);
	myheap->print();
	myheap->insert(100);
	myheap->print();
	myheap->insert(800);
	myheap->print();
	myheap->insert(200);
	myheap->print();
	myheap->insert(400);
	myheap->print();
	myheap->insert(900);
	myheap->print();
	myheap->insert(1000);
	myheap->print();
	myheap->insert(300);
	myheap->print();
	myheap->insert(600);
	myheap->print();

	// Get priority element from the heap
	int heapSize = myheap->size();
	for ( int i = 0; i < heapSize; i++ )
		cout << "Get min element = " << myheap->deletemin() << endl;

	// Cleanup
	delete myheap;
}
#endif
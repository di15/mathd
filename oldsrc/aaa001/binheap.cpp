

#include "binheap.h"
#include "pathfinding.h"

Heap::Heap()
{
	heap = NULL;
	allocsz = 0;
	nelements = 0;
}

Heap::~Heap()
{
	freemem();
}

void Heap::alloc(int ncells)
{
	freemem();

	heap = new TryStep* [ ncells ];
	allocsz = ncells;
	nelements = 0;
}

void Heap::freemem()
{
	if(heap)
	{
		delete [] heap;
		heap = NULL;
	}

	allocsz = 0;
	nelements = 0;
}

void Heap::resetelems()
{
	nelements = 0;
}

bool Heap::insert(TryStep* element)
{
    //heap.push_back(element);

	if(nelements >= allocsz)
		return false;

	heap[nelements] = element;
	nelements++;
    heapifyup(nelements - 1);

	return true;
}

bool Heap::hasmore()
{
	return nelements > 0;
}

TryStep* Heap::deletemin()
{
    TryStep* min = heap[0];
    heap[0] = heap[nelements - 1];
    nelements--;
    heapifydown(0);
    return min;
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
    return ( i < nelements ) ? i : -1;
}

int Heap::right(int parent)
{
    int i = ( parent << 1 ) + 2; // 2 * parent + 2
    return ( i < nelements ) ? i : -1;
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
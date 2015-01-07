

#ifndef LIST_H
#define LIST_H

// http://stackoverflow.com/questions/5136433/c-list-implementation

template <class T> class ListLink;
template <class T> class ListIt;	//iterator
template <class T> class ListRIt;	//reverse iterator

template <class T> 
class List
{
public:
	typedef ListIt<T> It;

	List();
	List(const List<T> & l);
	~List();

	void push_front(const T & x);
	void push_back(const T & x);
	void pop_front();
	void pop_back();
	void erase(It & pos); 
	It begin() const;
	It end() const;
	It rbegin() const;
	It rend() const;

	List<T>& operator=(const List<T>& l);

	ListLink<T> * head;
	ListLink<T> * tail;
	unsigned int size;
};

template <class T> 
class ListLink 
{
private:
	ListLink(const T & x): value(x), next(NULL), prev(NULL) {}//pg. 204 

	T value;     
	ListLink<T> * next;
	ListLink<T> * prev;

	friend class List<T>;
	friend class ListIt<T>;
};

template <class T>
List<T>::List()
{
	head = NULL;
	tail = NULL;
	size = 0;
}

template <class T>
List<T>::List(const List & l)
{
	head = 0;
	tail = 0;
	size = 0;
	for (ListLink<T> * current = l.head; current; current = current->next)
		push_back(current -> value);
}



#endif
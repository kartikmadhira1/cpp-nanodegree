#include <iostream>
#include <list>
#include <typeinfo>
#include <cstdlib>
#include "gc_details.h"
#include "gc_iterator.h"
/*
    Pointer implements a pointer type that uses
    garbage collection to release unused memory.
    A Pointer must only be used to point to memory
    that was dynamically allocated using new.
    When used to refer to an allocated array,
    specify the array size.
*/
template <class T, int size = 0>
class Pointer{
private:
    // refContainer maintains the garbage collection list.
    //this is a static list as it needs to keep track 
    //of every pointer initiated.
    static std::list<PtrDetails<T> > refContainer;
    // addr points to the allocated memory to which
    // this Pointer pointer currently points.
    T *addr;
    /*  isArray is true if this Pointer points
        to an allocated array. It is false
        otherwise. 
    */
    bool isArray; 
    // true if pointing to array
    // If this Pointer is pointing to an allocated
    // array, then arraySize contains its size.
    unsigned arraySize; // size of the array
    static bool first; // true when first Pointer is created
    // Return an iterator to pointer details in refContainer.
    typename std::list<PtrDetails<T> >::iterator findPtrInfo(T *ptr);
public:
    // Define an iterator type for Pointer<T>.
    typedef Iter<T> GCiterator;
    // Empty constructor
    // NOTE: templates aren't able to have prototypes with default arguments
    // this is why constructor is designed like this:
    Pointer(){
        Pointer(NULL);
    }
    Pointer(T*);
    // Copy constructor.
    Pointer(const Pointer &);
    // Destructor for Pointer.
    ~Pointer();
    // Collect garbage. Returns true if at least
    // one object was freed.
    static bool collect();
    // Overload assignment of pointer to Pointer.
    T *operator=(T *t);
    // Overload assignment of Pointer to Pointer.
    Pointer &operator=(Pointer &rv);
    // Return a reference to the object pointed
    // to by this Pointer.
    T &operator*(){
        return *addr;
    }
    // Return the address being pointed to.
    T *operator->() { return addr; }
    // Return a reference to the object at the
    // index specified by i.
    T &operator[](int i){ return addr[i];}
    // Conversion function to T *.
    operator T *() { return addr; }
    // Return an Iter to the start of the allocated memory.
    Iter<T> begin(){
        int _size;
        if (isArray)
            _size = arraySize;
        else
            _size = 1;
        return Iter<T>(addr, addr, addr + _size);
    }
    // Return an Iter to one past the end of an allocated array.
    Iter<T> end(){
        int _size;
        if (isArray)
            _size = arraySize;
        else
            _size = 1;
        return Iter<T>(addr + _size, addr, addr + _size);
    }
    // Return the size of refContainer for this type of Pointer.
    static int refContainerSize() { return refContainer.size(); }
    // A utility function that displays refContainer.
    static void showlist();
    // Clear refContainer when program exits.
    static void shutdown();
};

// STATIC INITIALIZATION
// Creates storage for the static variables
template <class T, int size>
std::list<PtrDetails<T> > Pointer<T, size>::refContainer;
template <class T, int size>
bool Pointer<T, size>::first = true;

// Constructor for both initialized and uninitialized objects. -> see class interface
template<class T,int size>
Pointer<T,size>::Pointer(T *t) {
    // Register shutdown() as an exit function.
    // Create a PtrDetails object and keep adding

    if(first)
        atexit(shutdown);
    first = false;
    if(size > 0) {
        isArray = true;
    }
    else {
        isArray = false;
    }
    // Setting the address to be at the new pointer's address
    addr = t;
    arraySize = size;
    typename std::list<PtrDetails<T>>::iterator p = findPtrInfo(t);
    PtrDetails<T> ptr_details;
    ptr_details.arraySize = size;
    ptr_details.memPtr = t;
    ptr_details.isArray = isArray;
    ptr_details.refcount++;
    if(p == refContainer.end()) {
        refContainer.push_back(ptr_details);
    }
    else {
        p->refcount++;
    }
}
// Copy constructor.
template< class T, int size>
Pointer<T,size>::Pointer(const Pointer &ob){
    // using iterator copy the fields of the object
    typename std::list<PtrDetails<T>>::iterator p = findPtrInfo(ob.addr);
    p->refcount++;
    addr = ob.addr;
    arraySize = ob.arraySize;
    if(arraySize > 0) isArray = true;
    else isArray = false;
}

// Destructor for Pointer.
/* Delete all the reference to the created pointers.
once the the pointer is out of scope, this destructor is called
since the memory this pointer is pointing might be shared by 
some other pointer as well, we just decrement the reference*/
template <class T, int size>
Pointer<T, size>::~Pointer() {
    typename std::list<PtrDetails<T>>::iterator p = findPtrInfo(addr);
    p->refcount--;
    collect();
}

// Collect garbage. Returns true if at least
// one object was freed.
template <class T, int size>
bool Pointer<T, size>::collect(){
    // Iterate through the list and check if 
    // the refCount on them is greater than 1.
    typename std::list<PtrDetails<T>>::iterator p = refContainer.begin();
    bool free_memory = false;
    while(p!=refContainer.end()) {
        if(p->refcount == 0) {
            // Also remove the object from the memory
            if(p->isArray) {
                delete[] p->memPtr;
            }
            else {
                delete p->memPtr;
            }
            // Remove PtrDetails object from the refContainer.
            free_memory = true;
            p = refContainer.erase(p);
        }
        else {
            p++;
        }
    }
    return free_memory;
}

// Overload assignment of pointer to Pointer.
template <class T, int size>
T *Pointer<T, size>::operator=(T *t){
    // If this object was previously pointing to another object
    // we delete the reference to that object
    typename std::list<PtrDetails<T>>::iterator p = findPtrInfo(addr);
    p->refcount--;
    if(size > 0) {
        isArray = true;
    }
    else {
        isArray = false;
    }
    // Setting the address to be at the new pointer's address
    addr = t;
    arraySize = size;
    // Use new PtrDetails object to point to a new address
    PtrDetails<T> ptr_details;
    ptr_details.arraySize = size;
    ptr_details.memPtr = t;
    ptr_details.isArray = isArray;
    // ptr_details.refcount++;
    p = findPtrInfo(t);
    if(p == refContainer.end()) {
        refContainer.push_back(ptr_details);
    }
    else {
        p->refcount++;
    }
    return addr;
}
// Overload assignment of Pointer to Pointer.
template <class T, int size>
Pointer<T, size> &Pointer<T, size>::operator=(Pointer &rv){
    // If this object was previously pointing to another object
    // we delete the reference to that object
    typename std::list<PtrDetails<T>>::iterator p = findPtrInfo(addr);
    p->refcount--;
    // Use new PtrDetails object to point to a new address
    PtrDetails<T> ptr_details;
    p = findPtrInfo(&rv);
    ptr_details.arraySize = rv.arraySize;
    ptr_details.memPtr = rv.memPtr;
    ptr_details.isArray = rv.isArray;
    // ptr_details.refcount++;

    if(p == refContainer.end()) {
        refContainer.push_back(ptr_details);
    }
    else {
        p->refcount++;
    }

    addr = p->memPtr;
    isArray = p->isArray;
    arraySize = p->arraySize;
    return *this;
}

// A utility function that displays refContainer.
template <class T, int size>
void Pointer<T, size>::showlist(){
    typename std::list<PtrDetails<T> >::iterator p;
    std::cout << "refContainer<" << typeid(T).name() << ", " << size << ">:\n";
    std::cout << "memPtr refcount value\n ";
    if (refContainer.begin() == refContainer.end())
    {
        std::cout << " Container is empty!\n\n ";
    }
    for (p = refContainer.begin(); p != refContainer.end(); p++)
    {
        std::cout << "[" << (void *)p->memPtr << "]"
             << " " << p->refcount << " ";
        if (p->memPtr)
            std::cout << " " << *p->memPtr;
        else
            std::cout << "---";
        std::cout << std::endl;
    }
    std::cout << std::endl;
}
// Find a pointer in refContainer.
template <class T, int size>
typename std::list<PtrDetails<T> >::iterator
Pointer<T, size>::findPtrInfo(T *ptr){
    typename std::list<PtrDetails<T> >::iterator p;
    // Find ptr in refContainer.
    for (p = refContainer.begin(); p != refContainer.end(); p++)
        if (p->memPtr == ptr)
            return p;
    return p;
}
// Clear refContainer when program exits.
template <class T, int size>
void Pointer<T, size>::shutdown(){
    if (refContainerSize() == 0)
        return; // list is empty
    typename std::list<PtrDetails<T> >::iterator p;
    for (p = refContainer.begin(); p != refContainer.end(); p++)
    {
        // Set all reference counts to zero
        p->refcount = 0;
    }
    collect();
}
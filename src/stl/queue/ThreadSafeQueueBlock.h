
#ifndef THREAD_SAFE_QUEUE_BLOCK_H_
#define THREAD_SAFE_QUEUE_BLOCK_H_

#include <memory>
#include <stdexcept>

namespace utils {
    
    template <typename T>
    class ThreadSafeQueueBlock {
    public: // usings
		using value_type	  = T;
		using pointer		  = T *;
		using const_pointer	  = const T *;
		using reference		  = T &;
		using const_reference = const T &;
		using size_type		  = size_t;
		using difference_type = std::ptrdiff_t; 

    private: // private methods
        ThreadSafeQueueBlock(value_type *mem_block, size_type capacity);
        ThreadSafeQueueBlock(size_type capacity);
        
        void clear();
        size_type checkAndAdd();

    public: // public methods
        ThreadSafeQueueBlock(const ThreadSafeQueueBlock &other);/////////////////
        ThreadSafeQueueBlock(ThreadSafeQueueBlock &&other);/////////////////
        ThreadSafeQueueBlock &operator=(const ThreadSafeQueueBlock &other);/////////////////
        ThreadSafeQueueBlock &operator=(ThreadSafeQueueBlock &&other);/////////////////
        
        ~ThreadSafeQueueBlock();

        size_type capacity() { return _capacity; }

        void push(const value_type &item);
        void push(value_type &&item);
        template <typename... Args>
		value_type &emplace(Args&&... args);

        template <typename U> 
        friend class ThreadSafeQueue;
    
    private: // class members
        volatile value_type *_mem_block;
        const size_type _capacity;
        volatile size_type _size;
        const bool _delete;
        volatile ThreadSafeQueueBlock<value_type> _next;
    };

    template <typename T>
    using safe_queue_block = ThreadSafeQueueBlock<T>;

    template <typename T>
    ThreadSafeQueueBlock<T> *make_safe_queue_block(typename ThreadSafeQueueBlock<T>::size_type capacity);
    template <typename T>
    void delete_safe_queue_block(ThreadSafeQueueBlock<T> *memory);
}

template <typename T>
utils::ThreadSafeQueueBlock<T> *make_safe_queue_block(typename utils::ThreadSafeQueueBlock<T>::size_type capacity) {
    using value_type = typename utils::ThreadSafeQueueBlock<T>::value_type;
    using obj = utils::ThreadSafeQueueBlock<T>;
    auto objSize = sizeof(obj);

    obj *build = ::operator new (objSize + capacity * sizeof(value_type));
	new(build) obj(build + objSize, capacity);

    return build;
}

template <typename T>
void delete_safe_queue_block(utils::ThreadSafeQueueBlock<T> *memory) {
    using value_type = typename utils::ThreadSafeQueueBlock<T>::value_type;

    auto capacity = memory->capacity();
    memory->~ThreadSafeQueueBlock();
    ::operator delete(memory, sizeof(utils::ThreadSafeQueueBlock<T>) + capacity * sizeof(value_type));
}


// Constructors: 

template <typename T>
utils::ThreadSafeQueueBlock<T>::ThreadSafeQueueBlock(value_type *mem_block, size_type capacity)
: _mem_block(mem_block), _capacity(capacity), _size(0), _delete(false), _next(nullptr) {}

template <typename T>
utils::ThreadSafeQueueBlock<T>::ThreadSafeQueueBlock(size_type capacity)
: _mem_block((value_type *) ::operator new (capacity * sizeof(value_type))), _capacity(capacity), 
_size(0), _delete(true), _next(nullptr) {}


// public methods:

template <typename T>
void utils::ThreadSafeQueueBlock<T>::push(const value_type &item) {
    auto place = checkAndAdd();
	new(&_mem_block[place]) value_type(item);
}

template <typename T>
void utils::ThreadSafeQueueBlock<T>::push(value_type &&item) {    
	auto place = checkAndAdd();
    new(&_mem_block[place]) value_type(std::move(item));
}

template <typename T> 
template <typename... Args> 
typename utils::ThreadSafeQueueBlock<T>::value_type &utils::ThreadSafeQueueBlock<T>::emplace(Args &&...args) {
	auto place = checkAndAdd();
    new(&_mem_block[place]) value_type(std::forward<Args>(args)...);

	return _mem_block[place];
}


// private methods:

template <typename T>
typename utils::ThreadSafeQueueBlock<T>::size_type utils::ThreadSafeQueueBlock<T>::checkAndAdd() {
    // no thread will be able to touch _mem_block[place], since you can only push back
    auto place = __atomic_fetch_add(&_size, 1, __ATOMIC_RELAXED);

    if (place == _capacity)
        throw std::out_of_range();

    return place;
}

template <typename T>
void utils::ThreadSafeQueueBlock<T>::clear() {
	for (size_type i = 0; i < _size; i++)
		_mem_block[i].~value_type();

	_size = 0;
}


// destructor:

template <typename T>
utils::ThreadSafeQueueBlock<T>::~ThreadSafeQueueBlock() {
    clear();

    if (_delete)
        ::operator delete(_mem_block, _capacity * sizeof(value_type));
}


#endif // !THREAD_SAFE_QUEUE_BLOCK_H_

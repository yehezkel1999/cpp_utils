
#ifndef THREAD_SAFE_QUEUE_BLOCK_H_
#define THREAD_SAFE_QUEUE_BLOCK_H_

#include <memory>
#include <stdexcept>

#include <thread>
#include <chrono>

#include <cstring> // for std::memcpy

namespace utils {
    
    template <typename T>
    class ThreadSafeQueue;

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
        void checkPeek(value_type *out);
        value_type &checkPeekRef();
        const size_type checkDequeueAndAdd();
        const size_type checkEnqueueAndAdd();

        size_type capacity() { return _capacity; }

        void push(const value_type &item);
        void push(value_type &&item);
        template <typename... Args>
		value_type &emplace(Args&&... args);

        value_type peek();
        value_type &peek_ref();
        void pop();
        value_type pop_get();
        
    public: // public methods
        // ThreadSafeQueueBlock(const ThreadSafeQueueBlock &other);/////////////////
        // ThreadSafeQueueBlock(ThreadSafeQueueBlock &&other);/////////////////
        // ThreadSafeQueueBlock &operator=(const ThreadSafeQueueBlock &other);/////////////////
        // ThreadSafeQueueBlock &operator=(ThreadSafeQueueBlock &&other);/////////////////
        
        ~ThreadSafeQueueBlock();

        friend class ThreadSafeQueue<value_type>;
    
    private: // class members
        volatile value_type *_mem_block;
        const size_type _capacity;
        volatile size_type _enqueue_i;
        volatile size_type _dequeue_i;
        const bool _delete;
        volatile ThreadSafeQueueBlock<value_type> _next; // TODO, who deletes _next? xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
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

    void *build = ::operator new (objSize + capacity * sizeof(value_type));
	new(reinterpret_cast<obj *>(build)) obj(reinterpret_cast<value_type *>(build + objSize), capacity);

    return reinterpret_cast<obj *>(build);
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
: _mem_block(mem_block), _capacity(capacity), _enqueue_i(0), _dequeue_i(0), _delete(false), _next(nullptr) {}

template <typename T>
utils::ThreadSafeQueueBlock<T>::ThreadSafeQueueBlock(size_type capacity)
: _mem_block(reinterpret_cast<value_type *>(::operator new (capacity * sizeof(value_type)))), _capacity(capacity), 
_enqueue_i(0), _delete(true), _next(nullptr) {}


// public methods:

template <typename T>
typename utils::ThreadSafeQueueBlock<T>::value_type utils::ThreadSafeQueueBlock<T>::peek() {
    // try to peek, if success then update _dequeue_i, else don't update and throw
    
    value_type *tempMem = reinterpret_cast<value_type *>(::operator new(sizeof(value_type)));

    checkPeek(tempMem);
    value_type temp(*tempMem);

    ::operator delete(tempMem, sizeof(value_type));

    return temp;
}

template <typename T>
typename utils::ThreadSafeQueueBlock<T>::value_type &utils::ThreadSafeQueueBlock<T>::peek_ref() {
    return checkPeekRef();
}


template <typename T>
void utils::ThreadSafeQueueBlock<T>::pop() {
    // since pop destroys the object, it mustn't let another thread peek/pop the same item,
    // so incriment first, then destroy

    const auto place = checkDequeueAndAdd();
    
    _mem_block[place].~value_type();
}

template <typename T>
typename utils::ThreadSafeQueueBlock<T>::value_type utils::ThreadSafeQueueBlock<T>::pop_get() {
    // since pop destroys the object, it mustn't let another thread peek/pop the same item,
    // so incriment first, then destroy

    const auto place = checkDequeueAndAdd();
    
    value_type temp(_mem_block[place]);
    _mem_block[place].~value_type();

    return temp;
}


template <typename T>
void utils::ThreadSafeQueueBlock<T>::push(const value_type &item) {
    const auto place = checkEnqueueAndAdd();
	new(&_mem_block[place]) value_type(item);
}

template <typename T>
void utils::ThreadSafeQueueBlock<T>::push(value_type &&item) {    
	const auto place = checkEnqueueAndAdd();
    new(&_mem_block[place]) value_type(std::move(item));
}

template <typename T> 
template <typename... Args> 
typename utils::ThreadSafeQueueBlock<T>::value_type &utils::ThreadSafeQueueBlock<T>::emplace(Args &&...args) {
	const auto place = checkEnqueueAndAdd();
    new(&_mem_block[place]) value_type(std::forward<Args>(args)...);

	return _mem_block[place];
}


// private methods:

template <typename T>
typename utils::ThreadSafeQueueBlock<T>::value_type &utils::ThreadSafeQueueBlock<T>::checkPeekRef() {
    // take enqueue before dequeue, so other threads won't be in the middle of enqueue, 
    // while dequeue check is happening
    auto enqueueI = __atomic_load_n(&_enqueue_i, __ATOMIC_RELAXED);
    auto dequeueI = __atomic_load_n(&_dequeue_i, __ATOMIC_RELAXED);
    
    if (dequeueI >= enqueueI)
        throw std::out_of_range();

    if (!__atomic_compare_exchange_n(&_dequeue_i, &dequeueI, dequeueI, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED)) {
        // a different thread changed _dequeue_i in between, sleep, then try again
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
        return checkPeekRef();
    }

    return &_mem_block[dequeueI];
}

template <typename T>
void utils::ThreadSafeQueueBlock<T>::checkPeek(value_type *out) {
    // take enqueue before dequeue, so other threads won't be in the middle of enqueue, 
    // while dequeue check is happening
    auto enqueueI = __atomic_load_n(&_enqueue_i, __ATOMIC_RELAXED);
    auto dequeueI = __atomic_load_n(&_dequeue_i, __ATOMIC_RELAXED);
    
    if (dequeueI >= enqueueI)
        throw std::out_of_range();

    std::memcpy(out, _mem_block[dequeueI], sizeof(size_type));

    if (!__atomic_compare_exchange_n(&_dequeue_i, &dequeueI, dequeueI, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED)) {
        // a different thread changed _dequeue_i in between, sleep, then try again
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
        checkPeek(out);
    }
}

template <typename T>
const typename utils::ThreadSafeQueueBlock<T>::size_type utils::ThreadSafeQueueBlock<T>::checkDequeueAndAdd() {
    // take enqueue before dequeue, so other threads won't be in the middle of enqueue, 
    // while dequeue check is happening
    auto enqueueI = __atomic_load_n(&_enqueue_i, __ATOMIC_RELAXED);
    auto dequeueI = __atomic_load_n(&_dequeue_i, __ATOMIC_RELAXED);
    
    if (dequeueI >= enqueueI)
        throw std::out_of_range();

    if (!__atomic_compare_exchange_n(&_dequeue_i, &dequeueI, dequeueI + 1, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED)) {
        // a different thread changed _dequeue_i in between, sleep, then try again
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
        return checkDequeueAndAdd();
    }

    return dequeueI;
}

template <typename T>
const typename utils::ThreadSafeQueueBlock<T>::size_type utils::ThreadSafeQueueBlock<T>::checkEnqueueAndAdd() {
    // no thread will be able to touch _mem_block[place], since you can only push back
    const auto place = __atomic_fetch_add(&_enqueue_i, 1, __ATOMIC_RELAXED);

    if (place >= _capacity)
        throw std::out_of_range();

    return place;
}



template <typename T>
void utils::ThreadSafeQueueBlock<T>::clear() {
	for (size_type i = _dequeue_i; i < _enqueue_i; i++)
		_mem_block[i].~value_type();

	__atomic_store_n(&_enqueue_i, 0, __ATOMIC_RELAXED);
	__atomic_store_n(&_dequeue_i, 0, __ATOMIC_RELAXED);
}

// destructor:

template <typename T>
utils::ThreadSafeQueueBlock<T>::~ThreadSafeQueueBlock() {
    clear();

    if (_delete)
        ::operator delete(_mem_block, _capacity * sizeof(value_type));
}


#endif // !THREAD_SAFE_QUEUE_BLOCK_H_

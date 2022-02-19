
#ifndef THREAD_SAFE_QUEUE_H_
#define THREAD_SAFE_QUEUE_H_

#include "ThreadSafeQueueBlock.h"

namespace utils {
    
    template <typename T>
    class ThreadSafeQueue {
    public: // usings
		using value_type	  = T;
		using pointer		  = T *;
		using const_pointer	  = const T *;
		using reference		  = T &;
		using const_reference = const T &;
		using size_type		  = size_t;
		using difference_type = std::ptrdiff_t;  
    private: // private usings
        using block = ThreadSafeQueueBlock<value_type>;

    private: // private methods

    public: // public methods
        ThreadSafeQueue();
        ~ThreadSafeQueue();

    private: // class members
        volatile block *_block;
    };

    template <typename T>
    using safe_queue = ThreadSafeQueue<T>;
}

template <typename T>
utils::ThreadSafeQueue<T>::ThreadSafeQueue() {}

template <typename T>
utils::ThreadSafeQueue<T>::~ThreadSafeQueue() {}

#endif // !THREAD_SAFE_QUEUE_H_

#pragma once
#include <iostream>
#include <deque>
#include <mutex>
#include <chrono>
#include <condition_variable>

/**
 * @brief A thread-safe, bounded multi-producer multi-consumer queue.
 *
 * This class provides a fixed-capacity queue supporting concurrent access
 * from multiple producers and consumers. It uses mutexes and condition variables
 * to synchronize access and supports both blocking and non-blocking operations. 
 * Timed push is also supported.
 *
 * @tparam T Type of elements stored in the queue.
 */
template<typename T>
class MPMCQueue
{
public:

	/**
	 * @brief Construct a new MPMCQueue with a given capacity.
	 * @param size_capacity Maximum number of elements the queue can hold.
	 */
	explicit MPMCQueue(size_t size_capacity) : m_MaxCapacity{size_capacity}
	{
	}

	
	// Disable copy and move semantics to prevent accidental sharing or moving of the queue
	MPMCQueue(const MPMCQueue&) = delete;
	MPMCQueue& operator=(const MPMCQueue&) = delete; 
	MPMCQueue(MPMCQueue&&) = delete;
	MPMCQueue& operator=(MPMCQueue&&) = delete;
	~MPMCQueue() = default;
	 
	/**
	 * @brief Push an element into the queue (blocking).
	 * Waits until space is available or the queue is closed.
	 * @param element Element to push (copied).
	 */
	void push(const T& element)
	{
		std::unique_lock lk {m_mutex};
		m_cv_not_full.wait(lk, [&] {
			return m_closed || m_queue.size() < m_MaxCapacity;
		});

		//Prevent adding elements while closed
		if(m_closed) return;
		m_queue.emplace_back(element);

		lk.unlock();
		//Let one random consumer that we have at least one element
		m_cv_not_empty.notify_one();
	}

	/**
	 * @brief Push an element into the queue using move semantics (blocking).
	 * Waits until space is available or the queue is closed.
	 * @param element Element to push (moved).
	 */
	void push(T&& element)
	{
		std::unique_lock<std::mutex> lk{ m_mutex };

		m_cv_not_full.wait(lk, [&]()
			{
				return m_closed || m_queue.size() < m_MaxCapacity;
			});

		if(m_closed) return;
		//Invoke movement to save performance on copies
		m_queue.emplace_back(std::move(element));

		lk.unlock();  
		m_cv_not_empty.notify_one();
	}

	/**
	 * @brief Push an element into the queue, waiting up to WaitTime for space.
	 * @param element Element to push (copied).
	 * @param WaitTime Maximum duration to wait for space.
	 */
	template<class Rep, class Period>
	void push_for(const T& element, std::chrono::duration<Rep, Period> WaitTime)
	{
		std::unique_lock lk{m_mutex};
		if (!m_cv_not_full.wait_for(lk, WaitTime, [&]()
		{
			return m_closed || m_queue.size() < m_MaxCapacity;
		}))
		{
			return;
		}

		if (m_closed) return;
		m_queue.emplace_back(element);

		lk.unlock();
		m_cv_not_empty.notify_one();
	}

	/**
	 * @brief Push an element into the queue using move semantics, waiting up to WaitTime for space.
	 * @param element Element to push (moved).
	 * @param WaitTime Maximum duration to wait for space.
	 */
	template<class Rep, class Period>
	void push_for(T&& element, std::chrono::duration<Rep,Period> WaitTime)
	{
		std::unique_lock<std::mutex> lk{m_mutex};

		if (!m_cv_not_full.wait_for(lk, WaitTime, [&]()
		{
			return m_closed || m_queue.size() < m_MaxCapacity;
		}))
		{
			return;
		}

		if(m_closed) return;
		m_queue.emplace_back(std::move(element));

		lk.unlock();
		m_cv_not_empty.notify_one();
	}

	/**
	 * @brief Try to push an element into the queue (non-blocking).
	 * @param element Element to push (copied).
	 * @return true if the element was pushed, false otherwise.
	 */
	bool try_push(const T& element)
	{
		std::scoped_lock<std::mutex> lockGuard{ m_mutex };
		if (m_closed || m_queue.size() >= m_MaxCapacity) return false;
		m_queue.emplace_back(element);
		return true;
	}

	/**
	 * @brief Try to push an element into the queue using move semantics (non-blocking).
	 * @param element Element to push (moved).
	 * @return true if the element was pushed, false otherwise.
	 */
	bool try_push(T&& element)
	{
		std::scoped_lock<std::mutex> lockGuard{ m_mutex };
		if (m_closed || m_queue.size() >= m_MaxCapacity) return false;
		m_queue.emplace_back(std::move(element));
		return true;
	}

	/**
	 * @brief Pop an element from the queue (blocking).
	 * Waits until an element is available or the queue is closed.
	 * @param element Reference to store the popped element.
	 */
	void pop(T& element)
	{
		std::unique_lock<std::mutex> lk{ m_mutex };

		m_cv_not_empty.wait(lk, [&]()
			{
				return m_closed || !m_queue.empty();
			});
		if(m_queue.empty()) return;

		element = std::move(m_queue.front());
		m_queue.pop_front();

		lk.unlock();
		m_cv_not_full.notify_one();
	}

	/**
	 * @brief Try to pop an element from the queue (non-blocking).
	 * @param element Reference to store the popped element.
	 * @return true if an element was popped, false otherwise.
	 */
	bool try_pop(T& element)
	{
		std::scoped_lock<std::mutex> lockGuard {m_mutex};
		if(m_queue.empty()) return false;
		
		//Otherwise if the type supports move semantics it's faster
		element = std::move(m_queue.front());
		m_queue.pop_front();
		
		return true;
	}

	/**
	 * @brief Outputs the contents of a snapshot of the queue to a stream (blocking).
	 * @param out Output stream.
	 * @param q Queue to print.
	 * @return Reference to the output stream.
	 */
	friend std::ostream& operator<<(std::ostream& out, const MPMCQueue& q)
	{
		std::deque<T> snapshot;
		std::unique_lock<std::mutex> lk{ q.m_mutex };
		snapshot = q.m_queue;
		lk.unlock();

		if (snapshot.empty()) 
		{
			out<<"[]\n";
			return out;
		}

		out<<'[';
		for (size_t i = 0; i < snapshot.size() - 1; ++i)
		{
			out<<snapshot[i]<<',';
		}
		out<< snapshot.back()<<"]\n";
		
		return out;
	}

	/**
	 * @brief Close the queue preventing further pushes and waking all waiting threads
	 */
	void close()
	{
		std::scoped_lock lk{m_mutex};
		//std::cout << "Closing queue\n";
		m_closed=true;
		m_cv_not_full.notify_all();
		m_cv_not_empty.notify_all();
	}

	/**
	 * @brief Get the maximum capacity of the queue.
	 * @return Maximum number of elements.
	 */
	inline size_t getMaxCapacity() const { return m_MaxCapacity; }

private:

	bool m_closed{ false }; ///< Indicates if the queue is closed to further pushes.
	const size_t m_MaxCapacity = 0; ///< Maximum number of elements allowed in the queue.
	std::deque<T> m_queue; ///< Underlying container for queue elements.
	mutable std::mutex m_mutex; ///< Mutex for synchronizing access.

	std::condition_variable m_cv_not_full; ///< Condition variable for waiting when the queue is full.
	std::condition_variable m_cv_not_empty; ///< Condition variable for waiting when the queue is empty.

};
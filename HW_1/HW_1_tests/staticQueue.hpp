
#ifndef OS_STATIC_QUEUE_H_
#define OS_STATIC_QUEUE_H_

#include <fstream>
#include <string>
#include <queue>

template<typename T>
class staticQueue{
private:
	const unsigned int 	m_MAX_SIZE;
	std::queue<T> 		m_queue;
	
public:
	staticQueue(int max_size):m_MAX_SIZE(max_size){}
	
	void push(const T& val){
		// staticQueue is not full - behaves like normal queue
		if (m_queue.size() < m_MAX_SIZE)
			m_queue.push(val);
		
		// staticQueue is full
		else{
			m_queue.pop();		// Remove oldest val
			m_queue.push(val);	// Insert new val
		}
	}
	
	T pop(){
		if (getSize() == 0)
			throw -1;
			
		T val = m_queue.front();
		m_queue.pop();		// Remove oldest val
		return val;
	}
	
	unsigned int getSize(){return m_queue.size();}

};

#endif /* OS_STATIC_QUEUE_H_ */


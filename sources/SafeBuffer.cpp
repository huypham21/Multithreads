#include "SafeBuffer.h"
#include <string>
#include <queue>
using namespace std;

SafeBuffer::SafeBuffer() {
	
}

SafeBuffer::~SafeBuffer() {
	
}

int SafeBuffer::size() {
	/*
	Is this function thread-safe???
	Make necessary modifications to make it thread-safe
	*/
    return q.size();
}

void SafeBuffer::push(string str) {
	mutex.lock()
	/*
	Is this function thread-safe???
	Make necessary modifications to make it thread-safe
	*/
	q.push (str);
		mutex.unlock()

}

string SafeBuffer::pop() {
	mutex.lock()

	/*
	Is this function thread-safe???
	Make necessary modifications to make it thread-safe
	*/

	string s = q.front();
	q.pop();
		mutex.unlock()

	return s;
}

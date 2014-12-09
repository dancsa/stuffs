/*
Copyright (c) 2014, GALAMBOS Dániel <dancsa@dancsa.hu>
Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
provided that the above copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT,
OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#ifndef CIRCULARBUFFER_HPP_INCLUDED
#define CIRCULARBUFFER_HPP_INCLUDED

#include <cstddef> //size_t
#include <string>


namespace dancsa {

class CircularBuffer;

/**
	@brief constans input iterator for CircularBuffer class
*/
class _const_circular_buffer_itr : public std::iterator<std::input_iterator_tag, char>{
	private:
		const CircularBuffer * buffer;
		std::size_t idx;

	public:
		_const_circular_buffer_itr( const CircularBuffer * _buffer, size_t _idx) : buffer(_buffer), idx(_idx){};
		_const_circular_buffer_itr( const _const_circular_buffer_itr &) = default;
		_const_circular_buffer_itr & operator=(const _const_circular_buffer_itr & o);

		bool operator==(const _const_circular_buffer_itr & o);
		bool operator!=(const _const_circular_buffer_itr & o);
		const char &  operator*();
		_const_circular_buffer_itr & operator++();
		_const_circular_buffer_itr operator++(int foo);
};



/**
	@brief Circular/ring buffer class for arbitary binary data.
	Data can be pushed back or poped from it.
*/
class CircularBuffer{
	private:
		char * buffer;
		std::size_t capacity_;
		std::size_t start_; //offsets from buffer
		std::size_t currsize_;
		
		std::size_t lastidx() const; //get the inner index of the last item. used internally

	public:
		/** typedef to input iterator */
		typedef _const_circular_buffer_itr const_iterator; 

		/**
			Constructor
			@param _size buffer size to be allocated.
		*/
		CircularBuffer(std::size_t _size) : capacity_(_size),start_(0),currsize_(0) { buffer = new char[currsize]; };
		CircularBuffer( const CircularBuffer& o);
		~CircularBuffer() { delete[] buffer; };
		std::size_t size() const;
		std::size_t availsize() const;
		std::size_t capacity() const;
		char & operator[](std::size_t idx);
		const char & operator[](std::size_t idx) const;
		CircularBuffer & operator=(const std::string &input);
		CircularBuffer & operator=(const char * input);
		CircularBuffer & operator+(const std::string  &input);
		CircularBuffer & operator+(const char * input);
		bool empty() const;
		void clear();
		void add(const void * buff, std::size_t len); 
		void pop(void * buff, std::size_t len);
		std::string pop_as_string(size_t len);
		const_iterator begin() const;
		const_iterator end() const;
};


} //namespace
#endif //include guard

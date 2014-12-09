/*
Copyright (c) 2014, GALAMBOS Dániel <dancsa@dancsa.hu>
Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
provided that the above copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT,
OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include <cstddef> //size_t
#include <stdexcept>
#include <string>

#include <cstring>
#include "circularbuffer.hpp"

using namespace dancsa;
using std::size_t;

/* _const_circular_buffer_itr impementation below */
_const_circular_buffer_itr & 
_const_circular_buffer_itr::operator=(const _const_circular_buffer_itr & o){
	this->buffer = o.buffer; 
	this->idx = o.idx;
}

bool 
_const_circular_buffer_itr::operator==(const _const_circular_buffer_itr & o){
	return ( this->buffer == o.buffer && this->idx == o.idx);
}

bool 
_const_circular_buffer_itr::operator!=(const _const_circular_buffer_itr & o){
	return !(*this == o);
}

_const_circular_buffer_itr & 
_const_circular_buffer_itr::operator++(){
	this->idx++;
	return *this;
}

_const_circular_buffer_itr 
_const_circular_buffer_itr::operator++(int foo){
	_const_circular_buffer_itr tmp(*this);
	this->operator++();
	return tmp;
}

const char & 
_const_circular_buffer_itr::operator*(){
	return (*(this->buffer))[idx];
}



/* CircularBuffer impementation below */

CircularBuffer::CircularBuffer( const CircularBuffer& o){
	this->currsize_ = o.currsize_;
	this->capacity_ = o.capacity_;
	this->start_ = o.start_;
	this->buffer = new char[o.capacity_];
	std::memcpy(this->buffer, o.buffer, this->capacity_);
}


/**
	@returns bytes available for reading from buffer
*/
size_t 
CircularBuffer::size() const{
	return this->currsize_;
}


/**
	Empties the buffer.
*/
void 
CircularBuffer::clear(){
	this->currsize_ = 0;
}


size_t 
CircularBuffer::lastidx() const { 
	if (0 == currsize_){ 
		return start_;
	}
	else{
		return ((start_+currsize_-1) % capacity_);
	}
}


/**
	@returns constant iterator to the first (oldest) byte
*/
CircularBuffer::const_iterator 
CircularBuffer::begin() const{
	return const_iterator(this, 0);
}


/**
	@returns constant iterator to after the lastest byte.
*/
CircularBuffer::const_iterator 
CircularBuffer::end() const {
	return const_iterator(this, this->size());	
}


/**
	@returns free bytes in the buffer
*/
size_t 
CircularBuffer::availsize() const{
	return ( this->capacity_ - this->currsize_);
}


/**
	@returns the size of the allocated buffer
*/
size_t 
CircularBuffer::capacity() const{
	return ( this->capacity_ - this->currsize_);
}


/**
	@returns true if and only if there are no data in the buffer
*/
bool 
CircularBuffer::empty() const{
	return ( 0 == this->currsize_);
}


/**
	@param idx index
	@returns reference to the indexth element's reference to modify the buffer
	This should be used for testing/debugging reasons only.
*/
char & 
CircularBuffer::operator[](std::size_t idx){
	if( idx >= this->currsize_){
		throw new std::out_of_range("overindexing");
	}
	return buffer[ (this->start_ + idx) % this->capacity_ ];
}


/**
	@param idx index
	@returns constant reference to the indexth element of the buffer (peeking in the puffer)
*/
const char & 
CircularBuffer::operator[](std::size_t idx) const {
	if( idx >= this->currsize_){
		throw new std::out_of_range("overindexing");
	}
	return buffer[ (this->start_ + idx) % this->capacity_ ];
}


/**
	@param input to overwrite the buffer with.
	Do NOT use this, if your string contains null byte, use clear() and add()
*/
CircularBuffer & 
CircularBuffer::operator=(const std::string &input){
	return this->operator=(input.c_str());
}


/**
	@param input c-string to overwrite the  buffer with.
	Note that the terminating null byte is NOT included.
*/
CircularBuffer & 
CircularBuffer::operator=(const char * input){
	size_t len = std::strlen(input);
	if( len > this->capacity_ ){
		throw new std::out_of_range("buffer can't handle this much data");
	}
	this->start_ = 0;
	this->currsize_ = len;
	std::memcpy(this->buffer, input, len);
	return *this;	
}


/**
	@param input append to buffer.
	@throws std::out_of_range if there are no enough space
	Do NOT use this, if your string contains null byte, use add()
*/
CircularBuffer & 
CircularBuffer::operator+(const std::string &input){
	return this->operator+(input.c_str());
}


/**
	@param input c-string, append to buffer
	@throws std::out_of_range if there are no enough space
	Note that the terminating null byte is NOT included.
*/
CircularBuffer & 
CircularBuffer::operator+(const char * input){
	size_t len = std::strlen(input);
	this->add(input, len);
	return *this;
}


/**
	@param buff address of data to be appended
	@param len length of data
	@throws std::out_of_range if there are no enough space
	Caller must ensure that there are enough free space in the buffer.
	Zero byte append is allowed.
*/
void
CircularBuffer::add(const void * buff, size_t len){
	if( len > this->availsize() ){
		throw new std::out_of_range("buffer can't handle or more than availsize data");
	}
	if ( 0 == len ){
		return;
	}
	using std::memcpy;

	size_t end = this->lastidx(); //last used byte's index
	if( end < start_ ){
		memcpy(buffer+end+1, buff, len);
	}else if(0 == currsize_){
		start_=0;
		memcpy(buffer, buff, len);
	}
	else{
		size_t free_at_end = this->capacity_ - end - 1;
		if( free_at_end >= len){ //will not roll over
			memcpy(buffer+end+1, buff, len);
		}else{
			memcpy(buffer+end+1, buff, free_at_end);
			memcpy(buffer, ((char*) buff)+free_at_end, len-free_at_end);
		}
	}
	currsize_ += len;
}


/**
	@param buff memory address to copy the data
	@param len length of the data to be extracted
	@throws std::out_of_range if more than available data requested
	Caller must not pop more data than the buffer current size.
*/
void 
CircularBuffer::pop(void * buff, size_t len){
	using std::memcpy;
	if ( 0 == len ){ //appending nothing can be valid.
		return;
	}
	if( len > this->currsize_ ){
		throw new std::out_of_range("cant get more data than size()");
	}
	
	size_t end = this->lastidx(); //last index
	if( end >= start_ ){
		memcpy(buff, buffer+start_, len);
		start_+=len;
		currsize_-=len;
	}else{
		memcpy(buff, buffer+start_, capacity_-start_);
		memcpy(((char*) buff)+(capacity_-start_), buffer, len-(capacity_-start_));
		start_ = (start_+len) % capacity_;
		currsize_-=len;
	}
}


/**
	Convience function to return data as std::string.
	@returns the requested data as std::string
	@throws std::out_of_range if more than available data requested
	Caller must not pop more data than the buffer current size.
*/
std::string
CircularBuffer::pop_as_string(size_t len){
	char * data = new char[len];
	this->pop(data, len);
	std::string ret(data,len);
	delete[] data;
	return ret;
}


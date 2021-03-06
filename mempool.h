#ifndef MEMPOOL_HH
#define MEMPOOL_HH

#include <cassert>
#include <iostream>
#include <vector>

class Mempool {
public:
    Mempool() : top(NULL), end(NULL) { }
    ~Mempool() {
	for (std::size_t i = 0; i < chunks.size(); i++)
	    delete[] chunks[i];
    }

    void* alloc(std::size_t n) {
	assert(n <= CHUNK_SIZE);
	// ensure alignment
	n += sizeof (void*) - 1;
	n -= n % sizeof (void*);
	if (top + n > end) {
	    chunks.push_back(new unsigned char[CHUNK_SIZE]);
	    top = chunks.back();
	    end = top + CHUNK_SIZE;
	}
	void* r = top;
	top += n;
	return r;
    }

    std::size_t mem_usage() {
	return CHUNK_SIZE * chunks.size() - (end - top);
    }

private:
    static const std::size_t CHUNK_SIZE = 1024 * 8;
    unsigned char *top, *end;
    std::vector<unsigned char*> chunks;
};

#endif	// MEMPOOL_HH

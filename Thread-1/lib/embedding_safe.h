#ifndef THREAD_LIB_EMBEDDING_SAFE_H_
#define THREAD_LIB_EMBEDDING_SAFE_H_

#include <string>
#include <mutex>
#include "embedding.h"

namespace proj1 {

class Embedding_safe : public Embedding {
public:
    Embedding_safe() : Embedding(), mux() {}
    Embedding_safe(int len) : Embedding(len), mux() {}
    Embedding_safe(int len, double* data) : Embedding(len, data), mux() {}
    Embedding_safe(int len, std::string str) : Embedding(len, str), mux() {}
    void lock() {this->mux.lock();}
    void unlock() {this->mux.unlock();}

private:
    std::mutex mux;
};



} // namespace proj1
#endif // THREAD_LIB_EMBEDDING_SAFE_H_

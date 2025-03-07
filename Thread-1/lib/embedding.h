#ifndef THREAD_LIB_EMBEDDING_H_
#define THREAD_LIB_EMBEDDING_H_

/**
 * Embedding:
 * Constructor and output method get lock automatically
 * get data, operaters will not get lock
 * 
 * only when do read-calc-write, should get wmux (then get mux when write),
 * so that thread only want to read also can read when calc,
 * thread also want to do read-calc-write cannot read
 * 
 * EmbeddingHolder: all methods are safe
*/

#include <string>
#include <vector>
#include <mutex>
#include <condition_variable>

namespace proj1 {

enum EMBEDDING_ERROR {
    LEN_MISMATCH = 0,
    NON_POSITIVE_LEN
};

class Embedding{
public:
    Embedding() {}
    Embedding(int);  // Random init an embedding
    Embedding(int, double*);
    Embedding(int, std::string);
    Embedding(Embedding*);
    Embedding(Embedding const&);
    ~Embedding() { delete []this->data; }
    double* get_data() { return this->data; }
    int get_length() { return this->length; }
    void update(Embedding*, double);
    std::string to_string();
    void write_to_stdout();
    // Operators
    Embedding operator+(Embedding&);
    Embedding operator+(const double);
    Embedding operator-(Embedding&);
    Embedding operator-(const double);
    Embedding operator*(Embedding&);
    Embedding operator*(const double);
    Embedding operator/(Embedding&);
    Embedding operator/(const double);
    bool operator==(Embedding&);
    //lock&unlock
    mutable std::mutex wmux;
    mutable std::mutex mux;
    void lock() {this->mux.lock();}
    void unlock() {this->mux.unlock();}

private:
    int length;
    double* data;
};

using EmbeddingMatrix = std::vector<Embedding*>;
using EmbeddingGradient = Embedding;

class EmbeddingHolder{
public:
    EmbeddingHolder(std::string filename);
    EmbeddingHolder(EmbeddingMatrix &data);
    ~EmbeddingHolder();
    static EmbeddingMatrix read(std::string);
    void write_to_stdout();
    void write(std::string filename);
    int append(Embedding *data);
    void update_embedding(int, EmbeddingGradient*, double);
    Embedding* get_embedding(int idx);
    unsigned int get_n_embeddings();
    int get_emb_length();
    bool operator==(EmbeddingHolder&);

private:
    EmbeddingMatrix emb_matx;
    // lock&unlock
    mutable std::mutex mux;
    // void lock() {this->mux.lock();}
    // void unlock() {this->mux.unlock();}
    mutable std::condition_variable cv;
};

} // namespace proj1
#endif // THREAD_LIB_EMBEDDING_H_

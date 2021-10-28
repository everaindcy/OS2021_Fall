#ifndef THREAD_LIB_EMBEDDING_H_
#define THREAD_LIB_EMBEDDING_H_

// Constructor and output method get lock automatically
// get data, update, operaters will not get lock

#include <string>
#include <vector>
#include <mutex>

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
    Embedding(Embedding&);
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
    std::mutex mux;
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
    Embedding* get_embedding(int idx) const { return this->emb_matx[idx]; } 
    unsigned int get_n_embeddings() { return this->emb_matx.size(); }
    int get_emb_length() {
        return this->emb_matx.empty()? 0: this->get_embedding(0)->get_length();
    }
    bool operator==(EmbeddingHolder&);
    // lock&unlock
    std::mutex mux;
    void lock() {this->mux.lock();}
    void unlock() {this->mux.unlock();}

private:
    EmbeddingMatrix emb_matx;
};

} // namespace proj1
#endif // THREAD_LIB_EMBEDDING_H_

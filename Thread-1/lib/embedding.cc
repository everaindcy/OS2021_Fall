
#include <fstream>
#include <iostream>
#include <sstream>
#include <cmath>
#include <mutex>

#include "utils.h"
#include "embedding.h"

namespace proj1 {

Embedding::Embedding(int length) {
    this->data = new double[length];
    for (int i = 0; i < length; ++i) {
        this->data[i] = (double) i / 10.0;
    }
    this->length = length;
}

Embedding::Embedding(int length, double* data) {
    embbedingAssert(length > 0, "Non-positive length encountered!", NON_POSITIVE_LEN);
    this->length = length;
    this->data = data;
}

Embedding::Embedding(Embedding* origin) {
    std::lock_guard<std::mutex> lock(origin->mux);
	int length = origin->get_length();
    embbedingAssert(length > 0, "Non-positive length encountered!", NON_POSITIVE_LEN);
    double* oldData = origin->get_data();
    double* newData = new double[length];
    for(int i = 0; i<length; i++) newData[i] = oldData[i];
    this->length = length;
    this->data = newData;
}

Embedding::Embedding(Embedding const& origin) {
	int length = origin.length;
    embbedingAssert(length > 0, "Non-positive length encountered!", NON_POSITIVE_LEN);
    double* oldData = origin.data;
    double* newData = new double[length];
    for(int i = 0; i<length; i++) newData[i] = oldData[i];
    this->length = length;
    this->data = newData;
}

Embedding::Embedding(int length, std::string raw) {
    embbedingAssert(length > 0, "Non-positive length encountered!", NON_POSITIVE_LEN);
    this->length = length;
    double* data = new double[length];
    std::stringstream ss(raw);
    int i;
    for (i = 0; (i < length) && (ss >> data[i]); ++i) {
        if (ss.peek() == ',')   ss.ignore();  // Ignore the delimiter
    }
    if (i < length) {
        delete []data;
        std::cerr << "Not enough elements in the input string!" << std::endl;
        throw LEN_MISMATCH;
    }
    this->data = data;
}

void Embedding::update(Embedding* gradient, double stepsize) {
    embbedingAssert(gradient->length == this->length,
           "Gradient has different length from the embedding!", LEN_MISMATCH);
    for (int i = 0; i < this->length; ++i) {
        this->data[i] -= stepsize * gradient->data[i];
    }
}

std::string Embedding::to_string() {
    std::lock_guard<std::mutex> lock(this->mux);

    std::string res;
    for (int i = 0; i < this->length; ++i) {
        if (i > 0) res += ',';
        res += std::to_string(this->data[i]);
    }
    return res;
}

void Embedding::write_to_stdout() {
    std::string prefix("[OUTPUT]");
    std::cout << prefix << this->to_string() << '\n';
}

Embedding Embedding::operator+(Embedding &another) {
    double* data = new double[this->length];
    for (int i = 0; i < this->length; ++i) {
        data[i] = this->data[i] + another.data[i];
    }
    return Embedding(this->length, data);
}

Embedding Embedding::operator+(const double value) {
    double* data = new double[this->length];
    for (int i = 0; i < this->length; ++i) {
        data[i] = this->data[i] + value;
    }
    return Embedding(this->length, data);
}

Embedding Embedding::operator-(Embedding &another) {
    double* data = new double[this->length];
    for (int i = 0; i < this->length; ++i) {
        data[i] = this->data[i] - another.data[i];
    }
    return Embedding(this->length, data);
}

Embedding Embedding::operator-(const double value) {
    double* data = new double[this->length];
    for (int i = 0; i < this->length; ++i) {
        data[i] = this->data[i] - value;
    }
    return Embedding(this->length, data);
}

Embedding Embedding::operator*(Embedding &another) {
    double* data = new double[this->length];
    for (int i = 0; i < this->length; ++i) {
        data[i] = this->data[i] * another.data[i];
    }
    return Embedding(this->length, data);
}

Embedding Embedding::operator*(const double value) {
    double* data = new double[this->length];
    for (int i = 0; i < this->length; ++i) {
        data[i] = this->data[i] * value;
    }
    return Embedding(this->length, data);
}

Embedding Embedding::operator/(Embedding &another) {
    double* data = new double[this->length];
    for (int i = 0; i < this->length; ++i) {
        data[i] = this->data[i] / another.data[i];
    }
    return Embedding(this->length, data);
}

Embedding Embedding::operator/(const double value) {
    double* data = new double[this->length];
    for (int i = 0; i < this->length; ++i) {
        data[i] = this->data[i] / value;
    }
    return Embedding(this->length, data);
}

bool Embedding::operator==(Embedding &another) {
    for (int i = 0; i < this->length; ++i) {
        if(fabs(this->data[i]-another.data[i])>1.0e-6)return false;
    }
    return true;
}

EmbeddingHolder::EmbeddingHolder(std::string filename) {
    this->emb_matx = this->read(filename);
}

EmbeddingHolder::EmbeddingHolder(std::vector<Embedding*> &data) {
    this->emb_matx = data;
}

EmbeddingMatrix EmbeddingHolder::read(std::string filename) {
    std::string line;
    std::ifstream ifs(filename);
    int length = 0;
    EmbeddingMatrix matrix;
    if (ifs.is_open()) {
        while (std::getline(ifs, line)) {
            if (length == 0) {
                for (char x: line) {
                    if (x == ',' || x == ' ')   ++length;
                }
                ++length;
            }
            Embedding* emb = new Embedding(length, line);
            matrix.push_back(emb);
        }
        ifs.close();
    } else {
        throw std::runtime_error("Error opening file " + filename + "!");
    }
    return matrix;
}

int EmbeddingHolder::append(Embedding* data) {
    std::lock_guard<std::mutex> lock(this->mux);

    int indx = this->emb_matx.size();
    embbedingAssert(
        indx == 0 | data->get_length() == this->emb_matx[0]->get_length(),
        "Embedding to append has a different length!", LEN_MISMATCH
    );
    this->emb_matx.push_back(data);

    this->cv.notify_all();
    return indx;
}

void EmbeddingHolder::write(std::string filename) {
    std::lock_guard<std::mutex> lock(this->mux);

    std::ofstream ofs(filename);
    if (ofs.is_open()) {
        for (Embedding* emb: this->emb_matx) {
            ofs << emb->to_string() + '\n';
        }
        ofs.close();
    } else {
        throw std::runtime_error("Error opening file " + filename + "!");
    }
}

void EmbeddingHolder::write_to_stdout() {
    std::lock_guard<std::mutex> lock(this->mux);

    std::string prefix("[OUTPUT]");
    for (Embedding* emb: this->emb_matx) {
        std::cout << prefix + emb->to_string() + '\n';
    }
}

EmbeddingHolder::~EmbeddingHolder() {
    for (Embedding* emb: this->emb_matx) {
        delete emb;
    }
}

void EmbeddingHolder::update_embedding(int idx, EmbeddingGradient* gradient, double stepsize) {
    Embedding* emb = this->get_embedding(idx);
    emb->lock();
    emb->update(gradient, stepsize);
    emb->unlock();
}

Embedding* EmbeddingHolder::get_embedding(int idx) {
    std::unique_lock<std::mutex> lock(this->mux);

    while (idx >= this->emb_matx.size()) {
        this->cv.wait(lock);
    }
    return this->emb_matx[idx];
}

unsigned int EmbeddingHolder::get_n_embeddings() {
    std::lock_guard<std::mutex> lock(this->mux);

    return this->emb_matx.size();
}

int EmbeddingHolder::get_emb_length() {
    std::lock_guard<std::mutex> lock(this->mux);

    return this->emb_matx.empty()? 0: this->emb_matx[0]->get_length();
}

bool EmbeddingHolder::operator==(EmbeddingHolder &another) {
    std::lock_guard<std::mutex> lock(this->mux);
    std::lock_guard<std::mutex> lock1(another.mux);

    if (this->emb_matx.size() != another.emb_matx.size())
        return false;

    for (int i = 0; i < (int)this->emb_matx.size(); ++i) {
        std::lock_guard<std::mutex> lock2(this->emb_matx[i]->mux);
        std::lock_guard<std::mutex> lock3(another.emb_matx[i]->mux);
        if(!(*(this->emb_matx[i]) == *(another.emb_matx[i]))){
        	return false;
		}
    }
    return true;
}

} // namespace proj1

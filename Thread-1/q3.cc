#include <vector>
#include <tuple>

#include <string>   // string
#include <chrono>   // timer

#include "lib/utils.h"
#include "lib/server.h"
#include "lib/instruction.h"

int main(int argc, char *argv[]) {

    proj1::Server server("data/q3.in", "data/q3.in", true, true, true);

    proj1::Instructions instructions = proj1::read_instructrions("data/q3_instruction.tsv");

    {
        proj1::AutoTimer timer("q3");  // using this to print out timing of the block
        // Run all the instructions
        
        std::vector<std::thread*> threads;

        for (proj1::Instruction inst: instructions) {
            auto t = new std::thread([&server, inst](){server.do_instruction(inst);});
            threads.push_back(t);
        }
        for (auto t: threads) {
            t->join();
            delete t;
        }
    }

    // Write the result
    server.write_to_stdout();

    return 0;
}

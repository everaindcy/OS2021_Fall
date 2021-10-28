#include <vector>
#include <tuple>

#include <string>   // string
#include <chrono>   // timer

#include "lib/utils.h"
#include "lib/server.h"
#include "lib/instruction.h"

int main(int argc, char *argv[]) {

    proj1::Server server("data/q2.in", "data/q2.in", true, true, false);

    proj1::Instructions instructions = proj1::read_instructrions("data/q2_instruction.tsv");

    {
        proj1::AutoTimer timer("q2");  // using this to print out timing of the block
        // Run all the instructions
        for (proj1::Instruction inst: instructions) {
            server.do_instruction(inst);
        }
    }

    // Write the result
    server.write_to_stdout();

    return 0;
}

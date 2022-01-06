#include <gtest/gtest.h>
#include <vector>

#include "server.h"
#include "embedding.h"

namespace proj1 {
namespace testing {

class ServerTest : public ::testing::Test {
protected:
    void SetUp() {
        server1 = new Server("data/q4.in", "data/q4.in");
        server2 = new Server("data/q4.in", "data/q4.in");
        server3 = new Server("data/q4.in", "data/q4.in");

        inst_init = new Instruction("0 0 1 2 3 4 5 6 7 8");
        inst_update = new Instruction("1 7 5 0 0");
        inst_recommend = new Instruction("2 2 -1 4 5 6 3 7 9");
    }
    Server *server1, *server2, *server3;
    Instruction *inst_init, *inst_update, *inst_recommend;
};

TEST_F(ServerTest, test_init) {
    server1->do_init(*inst_init);
    server2->do_init_safe(*inst_init);
    EXPECT_EQ((*server1) == (*server2), true);
}

TEST_F(ServerTest, test_update) {
    server1->do_update(*inst_update);
    server2->do_update_safe(*inst_update);
    server3->do_update_epoch(*inst_update);
    EXPECT_EQ((*server1) == (*server2), true);
    EXPECT_EQ((*server2) == (*server3), true);
}

TEST_F(ServerTest, test_recommend) {
    Embedding* emb1 = server1->do_recommend(*inst_recommend);
    Embedding* emb2 = server2->do_recommend_safe(*inst_recommend);
    Embedding* emb3 = server3->do_recommend_epoch(*inst_recommend);
    EXPECT_EQ((*emb1) == (*emb2), true);
    EXPECT_EQ((*emb2) == (*emb3), true);
}
    
} // namespace testing
} // namespace proj1

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

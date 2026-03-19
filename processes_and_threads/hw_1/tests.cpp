#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <functional>
#include <algorithm>

#include "apply_function.hpp"

TEST(ApplyFunctionTest, SingleThread) {
    std::vector<int> data = {1, 2, 3, 4, 5};
    std::function<void(int&)> transform = [](int& x) { x *= 2; };
    ApplyFunction(data, transform, 1);
    EXPECT_EQ(data, std::vector<int>({2, 4, 6, 8, 10}));
}

TEST(ApplyFunctionTest, MultiThread) {
    std::vector<int> data = {1, 2, 3, 4, 5};
    std::function<void(int&)> transform = [](int& x) { x *= 2; };
    ApplyFunction(data, transform, 3);
    EXPECT_EQ(data, std::vector<int>({2, 4, 6, 8, 10}));
}

TEST(ApplyFunctionTest, ThreadsExceedSize) {
    std::vector<int> data = {1, 2, 3};
    std::function<void(int&)> transform = [](int& x) { x += 1; };
    ApplyFunction(data, transform, 10);
    EXPECT_EQ(data, std::vector<int>({2, 3, 4}));
}

TEST(ApplyFunctionTest, EmptyVector) {
    std::vector<int> data;
    std::function<void(int&)> transform = [](int& x) { x *= 2; };
    ApplyFunction(data, transform, 5);
    EXPECT_TRUE(data.empty());
}

TEST(ApplyFunctionTest, DifferentType) {
    std::vector<std::string> data = {"hello", "world"};
    std::function<void(std::string&)> transform = 
        [](std::string& s) { std::reverse(s.begin(), s.end()); };
    ApplyFunction(data, transform, 2);
    EXPECT_EQ(data, std::vector<std::string>({"olleh", "dlrow"}));
}

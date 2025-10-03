#include <gtest/gtest.h>

#include "lib/interpreter/interpreter.h"

struct InterpreterTestParams {
    std::string code;
    std::string expected;
};

class InterpreterTestSuite 
    : public ::testing::TestWithParam<InterpreterTestParams> {};

TEST_P(InterpreterTestSuite, CheckTokensForEquality) {
    const auto& [code, expected] = GetParam();
    std::istringstream input(code);
    std::ostringstream output;
    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}

INSTANTIATE_TEST_SUITE_P(
    AllPossibleCasesTestSuite,
    InterpreterTestSuite,
    ::testing::Values(
        InterpreterTestParams{
            .code = R"(
                a = 1
                b = 2
                c = a + b
                print(c)
            )",
            .expected = "3"
        },
        InterpreterTestParams{
            .code = R"(
                if true then
                    print("true")
                end if
            )",
            .expected = "true"
        },
        InterpreterTestParams{
            .code = R"(
                if 1 < 0 then
                    print("true")
                else
                    print("false")
                end if
            )",
            .expected = "false"
        },
        InterpreterTestParams{
            .code = R"(
                a = ""
                for i in range(1, 10, 1)
                    a += to_string(i)
                end for
                print(a)
            )",
            .expected = "123456789"
        },
        InterpreterTestParams{
            .code = R"(
                a = "ITMOITMO"
                print(a[1:8])
            )",
            .expected = "TMOITMO"
        },
        InterpreterTestParams{
            .code = R"(
                n = 10
                res = 1
                for i in range(1, n + 1, 1)
                    res *= i
                end for
                print(res)
            )",
            .expected = "3628800"
        }
    )
);


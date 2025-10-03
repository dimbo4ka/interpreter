#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "lib/interpreter/interpreter.h"


std::string kUnreachable = "239";


TEST(IllegalOperationsSuite, TypeMixing) {
    std::vector<std::string> values = {
        "123",
        "\"string\"",
        "[1, 2, 3]",
        "function() end function",
        "nil",
    };

    for (int a = 0; a < values.size(); ++a) {
        for (int b = a + 1; b < values.size(); ++b) {
            std::stringstream input;
            input << "a = " << values[a] << "\n";
            input << "b = " << values[b] << "\n";
            input << "c = a + b" << "\n";
            input << "print(239) // unreachable" << "\n";

            std::ostringstream output;

            ASSERT_FALSE(interpret(input, output));
            ASSERT_FALSE(output.str().ends_with(kUnreachable));
        }
    }
}


TEST(IllegalOperationsSuite, ArgumentCountMismatch) {
    std::string code = R"(
        func = function(value) return 1 end function

        func(1, 2)

        print(239) // unreachable
    )";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_FALSE(interpret(input, output));
    ASSERT_FALSE(output.str().ends_with(kUnreachable));
}

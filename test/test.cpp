#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include <iostream>

// change if you choose to use a different header name
#include "CampusCompass.h"

using namespace std;

#include <sstream>
#include "CampusCompass.h"

// Helper: run the program logic on a given `input` string and
// return everything it prints to cout as a single string.
static string runInput(const string &input) {
    stringstream outputBuffer;
    streambuf *oldBuf = cout.rdbuf(outputBuffer.rdbuf());

    {
        CampusCompass c;
        c.ParseCSV("../data/edges.csv", "../data/classes.csv");

        istringstream in(input);
        int n;
        in >> n;
        in.ignore();

        string line;
        for (int i = 0; i < n; ++i) {
            getline(in, line);
            c.ParseCommand(line);
        }
    }

    cout.rdbuf(oldBuf);

    return outputBuffer.str();
}


// the syntax for defining a test is below. It is important for the name to be
// unique, but you can group multiple tests with [tags]. A test can have
// [multiple][tags] using that syntax.
TEST_CASE("Example Test Name - Change me!", "[tag]") {
  // instantiate any class members that you need to test here
  int one = 1;

  // anything that evaluates to false in a REQUIRE block will result in a
  // failing test
  REQUIRE(one == 1); // fix me!

  // all REQUIRE blocks must evaluate to true for the whole test to pass
  REQUIRE(true); // also fix me!
}

TEST_CASE("Test 2", "[tag]") {
  // you can also use "sections" to share setup code between tests, for example:
  int one = 1;

  SECTION("num is 2") {
    int num = one + 1;
    REQUIRE(num == 2);
  };

  SECTION("num is 3") {
    int num = one + 2;
    REQUIRE(num == 3);
  };

  // each section runs the setup code independently to ensure that they don't
  // affect each other
}

// You must write 5 unique, meaningful tests for credit on the testing section
// of this project!

// See the following for an example of how to easily test your output.
// Note that while this works, I recommend also creating plenty of unit tests for particular functions within your code.
// This pattern should only be used for final, end-to-end testing.

// This uses C++ "raw strings" and assumes your CampusCompass outputs a string with
//   the same thing you print.
TEST_CASE("Example CampusCompass Output Test", "[flag]") {
  // the following is a "raw string" - you can write the exact input (without
  //   any indentation!) and it should work as expected
  // this is based on the input and output of the first public test case
  string input = R"(6
insert "Student A" 10000001 1 1 COP3502
insert "Student B" 10000002 1 1 COP3502
insert "Student C" 10000003 1 2 COP3502 MAC2311
dropClass 10000001 COP3502
remove 10000001
removeClass COP3502
)";

  string expectedOutput = R"(successful
successful
successful
successful
unsuccessful
2
)";

  // somehow pass your input into your CampusCompass and parse it to call the
  // correct functions, for example:
  /*
  CampusCompass c;
  c.parseInput(input)
  // this would be some function that sends the output from your class into a string for use in testing
  actualOutput = c.getStringRepresentation()
  */
  string actualOutput = runInput(input);

  REQUIRE(actualOutput == expectedOutput);
}

TEST_CASE("CampusCompass Output Test 1") {
  string input = R"(6
insert "Student A" 10000001 1 1 COP3502
insert "Student A Copy" 10000001 1 1 COP3502
insert "Student B" 10000002 1 1 COP3502
remove 10000001
remove 10000001
removeClass COP3502
)";

  std::string expectedOutput = R"(successful
unsuccessful
successful
successful
unsuccessful
1
)";

  string actualOutput = runInput(input);

  REQUIRE(actualOutput == expectedOutput);
}

TEST_CASE("CampusCompass Output Test Given Example") {
  string input = R"(7
insert "Brandon" 45679999 20 2 COP3530 MAC2311
insert "Brian" 35459999 21 3 COP3530 CDA3101 MAC2311
insert "Briana" 87879999 22 3 CDA3101 MAC2311 EEL3701
removeClass COP3530
remove 87879999 
removeClass CDA3101
printShortestEdges 35459999
)";

  std::string expectedOutput = R"(successful
successful
successful
2
successful
1
Name: Brian
MAC2311 | Total Time: 11
)";

  string actualOutput = runInput(input);

  REQUIRE(actualOutput == expectedOutput);
}

TEST_CASE("CampusCompass Output Test 2") {
    std::string input = R"(5
insert "A11y" 45679999 1 1 COP3530
insert "Alice" 1234567 1 1 COP3530
dropClass abcdefgh COP3530
replaceClass 12345678 COP3530 BADCODE
togglEdgesClosure 1 1 2
)";

    std::string expectedOutput =
R"(unsuccessful
unsuccessful
unsuccessful
unsuccessful
unsuccessful
)";

    REQUIRE(runInput(input) == expectedOutput);
}

TEST_CASE("CampusCompass Output Test 3") {
    std::string input = R"(3
remove 99999999
dropClass 12345678 COP3530
replaceClass 87654321 COP3530 MAC2311
)";

    std::string expectedOutput =
R"(unsuccessful
unsuccessful
unsuccessful
)";

    REQUIRE(runInput(input) == expectedOutput);
}

TEST_CASE("CampusCompass Output Test 4") {
    std::string input = R"(8
insert "Alice" 10000001 1 2 COP3530 MAC2311
replaceClass 10000001 COP3530 EEL3701
dropClass 10000001 MAC2311
removeClass EEL3701
remove 10000001
remove 10000001
dropClass 10000001 EEL3701
replaceClass 10000001 EEL3701 COP3530
)";

    std::string expectedOutput =
R"(successful
successful
successful
1
unsuccessful
unsuccessful
unsuccessful
unsuccessful
)";

    REQUIRE(runInput(input) == expectedOutput);
}

TEST_CASE("CampusCompass Output Test 5") {

    std::string input = R"(4
insert "Alice" 10000001 1 1 COP3530
printShortestEdges 10000001
toggleEdgesClosure 3 1 2 1 4 1 50
printShortestEdges 10000001
)";

    std::string out = runInput(input);

    auto firstPos = out.find("COP3530 | Total Time:");
    REQUIRE(firstPos != std::string::npos);
    auto firstEnd = out.find('\n', firstPos);
    if (firstEnd == std::string::npos) firstEnd = out.size();
    std::string line1 = out.substr(firstPos, firstEnd - firstPos);

    auto secondPos = out.find("COP3530 | Total Time:", firstEnd);
    REQUIRE(secondPos != std::string::npos);
    auto secondEnd = out.find('\n', secondPos);
    if (secondEnd == std::string::npos) secondEnd = out.size();
    std::string line2 = out.substr(secondPos, secondEnd - secondPos);

    REQUIRE(line1.find("Total Time: -1") == std::string::npos);

    REQUIRE(line2.find("Total Time: -1") != std::string::npos);
}


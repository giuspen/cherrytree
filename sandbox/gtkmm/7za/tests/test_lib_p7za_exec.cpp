
// https://cpputest.github.io/
// sudo apt install cpputest
// g++ test_lib_p7za_exec.cpp -o bin_test_lib_p7za_exec ../libp7za.a `pkg-config cpputest glibmm-2.4 --cflags --libs`

#include "CppUTest/CommandLineTestRunner.h"


extern int p7za_exec(int numArgs, char *args[]);


TEST_GROUP(P7zaExecGroup)
{
};


TEST(P7zaExecGroup, extract)
{
    
}




int main(int ac, char** av)
{
    return CommandLineTestRunner::RunAllTests(ac, av);
}

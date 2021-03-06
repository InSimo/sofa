#include <sofa/helper/assert.h>

#include <iostream>

namespace sofa
{
namespace assertion
{

//////////////////////////////////////////////////
Action defaultAssertionHandler(const char* condition, const char* file, unsigned int line, const char* msg)
{
    std::cout << file << ":" << line << ": Assertion failure";

    if (condition != NULL) {
        std::cout << ": " << condition;
    }

    if (msg != NULL) {
        std::cout << ", \"" << msg << "\"";
    }

    std::cout << std::endl;

    return HALT;
}

} // namespace assertion
} // namespace sofa

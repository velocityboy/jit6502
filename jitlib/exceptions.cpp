#include "stdafx.h"

#include "Exceptions.h"

#include <stdexcept>
#include <sstream>

using std::ios_base;
using std::ostringstream;
using std::runtime_error;

ios_base &throwError(ios_base &stm)
{
    auto *poss = dynamic_cast<ostringstream*>(&stm);
    if (poss != nullptr) {
        throw runtime_error(poss->str());
    }

    throw runtime_error("Bad runtime_error constructor");
}


/******************************************************************************
*       SOFA, Simulation Open-Framework Architecture, version 1.0 RC 1        *
*            (c) 2006-2021 INRIA, USTL, UJF, CNRS, MGH, InSimo                *
*                                                                             *
* This library is free software; you can redistribute it and/or modify it     *
* under the terms of the GNU Lesser General Public License as published by    *
* the Free Software Foundation; either version 2.1 of the License, or (at     *
* your option) any later version.                                             *
*                                                                             *
* This library is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License *
* for more details.                                                           *
*                                                                             *
* You should have received a copy of the GNU Lesser General Public License    *
* along with this library; if not, write to the Free Software Foundation,     *
* Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.          *
*******************************************************************************
*                              SOFA :: Framework                              *
*                                                                             *
* Authors: The SOFA Team (see Authors.txt)                                    *
*                                                                             *
* Contact information: contact@sofa-framework.org                             *
******************************************************************************/
#include <sofa/helper/vector.h>
#include <sofa/helper/vector_device.h>
#include <sofa/helper/integer_id.h>
#include <sofa/helper/Factory.h>
#include <sofa/helper/BackTrace.h>
#include <sofa/helper/system/config.h>
#include <cassert>
#include <iostream>
#include <stdlib.h>

namespace sofa
{

namespace helper
{

#if defined(_XBOX) || defined(__PS3__)
static char* getenv(const char* varname) { return NULL; } // NOT IMPLEMENTED
#endif

#ifdef DEBUG_OUT_VECTOR
int cptid = 0;
#endif

bool vector_access_call_assert()
{
    const char* val = getenv("SOFA_DEBUG_VECTOR_ASSERT");
    return (val == NULL || (atoi(val) != 0));
}

void SOFA_HELPER_API vector_access_failure(const void* vec, unsigned size, unsigned i, const std::type_info& type)
{
    std::cerr << "ERROR in vector<"<<gettypename(type)<<"> " << std::hex << (std::uintptr_t)vec << std::dec << " size " << size << " : invalid index " << (int)i << std::endl;
    BackTrace::dump();
    static bool do_assert = vector_access_call_assert();
    if (do_assert)
    {
        assert(i < size);
    }
}

void SOFA_HELPER_API vector_access_failure(const void* vec, unsigned size, unsigned i, const std::type_info& type, const char* tindex)
{
    std::cerr << "ERROR in vector<"<<gettypename(type)<<", integer_id<"<<tindex<<"> > " << std::hex << (std::uintptr_t)vec << std::dec << " size " << size << " : invalid index " << (int)i << std::endl;
    BackTrace::dump();
    static bool do_assert = vector_access_call_assert();
    if (do_assert)
    {
        assert(i < size);
    }
}

} // namespace helper

} // namespace sofa

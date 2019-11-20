/*
==== Author:

Relja Arandjelovic (relja@robots.ox.ac.uk)
Visual Geometry Group,
Department of Engineering Science
University of Oxford

==== Copyright:

The library belongs to Relja Arandjelovic and the University of Oxford.
No usage or redistribution is allowed without explicit permission.
*/

#ifndef _MACROS_H_
#define _MACROS_H_

#include <iostream>
#include <cstdlib>

// assert which works in Release mode too
#define ASSERT(expression) if (!(expression)) { std::cerr << "ASSERT failed: " #expression " in "  << __FUNCTION__ << " (" __FILE__ ":" << __LINE__ << ")\n"; exit(1); }

// apply assert and evaluate expression only for debugging
#if 0
#define DEBUGASSERT(expression) ASSERT(expression)
#else
#define DEBUGASSERT(expression)
#endif


// disallow copy constructor and assignment operator
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName&); \
    void operator=(const TypeName&);


// to avoid the warning about not using a variable (useful when doing pread64 as there is a warning if output is not checked)
#define REMOVE_UNUSED_WARNING(temp_) if (false && temp_) {}

#endif

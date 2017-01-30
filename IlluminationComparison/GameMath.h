#pragma once

// For experience lets try and keep this with the ability to choose our math library.
// This will be a fun challenge and will also allow me to switch between math libraries
// to test preformance!
#define WITH_DX

#ifdef WITH_DX
#include "DXMathImpl.h"
#else
#error "Unrecognized math system!"
#endif // WITH_DX
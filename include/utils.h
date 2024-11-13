#pragma once
#include <stdio.h>
#include <cassert>

#define Assert(exp, msg) if(!(exp)){printf("%s\n", msg); assert(0);}

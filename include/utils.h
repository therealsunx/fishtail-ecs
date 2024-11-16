#pragma once

#define TR_ASSERT

#ifdef TR_ASSERT
#   include <assert.h>
#   define Assert(exp, msg) if(!(exp)){printf("\x1b[31m%s\x1b[m\n", msg); assert(0);}
#else
#   define Assert(exp, msg)
#endif

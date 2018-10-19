#pragma once

//a helper macro to deal with there being no drand48 on windows
#if _WIN64 || _WIN32
#define drand48() (float)(((float)rand())/((float)RAND_MAX))
#endif
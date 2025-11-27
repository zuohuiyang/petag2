#pragma once
#include <cstdlib>

#define CHECK_LE(a,b) do { if (!((a) <= (b))) std::abort(); } while (0)
#define CHECK_GT(a,b) do { if (!((a) > (b))) std::abort(); } while (0)


#include "randomutil.h"
#include <time.h>
#include <cstdlib>

float RandomBetween(float min, float max)
{
    static bool randInitialized = false;
    if (!randInitialized)
    {
        srand(time(NULL));
        randInitialized = true;
    }

    const size_t divisor = 10000;
    return min + ((rand() % divisor) / static_cast<float>(divisor)) * (max - min);
}

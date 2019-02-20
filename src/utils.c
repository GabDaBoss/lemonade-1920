#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

static bool initiated;


double
utils_random()
{
    if(!initiated) {
        srand(time(NULL));
        initiated = !initiated;
    }

    double r = (double) rand() / (double) rand();

    return r - (int) r;
}

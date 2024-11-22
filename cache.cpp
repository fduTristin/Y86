#include "y86.h"
#include "cache.h"

int main()
{
    initialize_cache(8, 4, 8, cache);
    input();
    while (CPU.state == 1)
    {
        decode_and_execute(cache);
        create_output();
    }
    output();
}

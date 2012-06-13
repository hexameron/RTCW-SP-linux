#include <unistd.h>
#include <string.h>

/* Could use Hardware DMA here */

void Com_Memcpy( void* dest, const void* src, const size_t count ) {
         memcpy( dest, src, count );
         }
void Com_Memset( void* dest, const int val, const size_t count ) {
         memset( dest, val, count );
         }

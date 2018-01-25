#include <string.h>
#include "cache.h"


static size_t cacheSize = 0;


int findInCache(char *url)
{

    for (size_t j = 0; j < cacheSize; ++j)
    {
        //if (strcmp(cache[j].url, url) == 0 && cache[j].entryStatus != ES_INVALID)
        //{
            return j;
        //}
    }

    return -1;
}

int getFreeCacheIndex()
{
    if(cacheSize < CACHE_SIZE){
        int ret = cacheSize;
        cacheSize++;
        return ret;
    }

    for (size_t j = 0; j < cacheSize; ++j)
    {
        //if(cache[j].entryStatus == ES_INVALID){
          //  return j;
        //}
    }

    return -1;

}

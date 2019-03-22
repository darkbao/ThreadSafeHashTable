/*
* @Author:             darkbao
* @Date:               2019-03-22 17:02:10
* @Last Modified by:   darkbao
* @Email:              freecare_j@163.com
*/
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include "ThreadSafeHashTable.hpp"

using namespace std;
using namespace mj;
typedef ThreadSafeHashTable<int, std::string> Int2StrHashMap;
Int2StrHashMap hashMap;

auto print = [](const pair<int, string>& valueType) {
    printf("[%d]->[%s] ", valueType.first, valueType.second.c_str());
};

void* threadFunc(void* arg)
{
    int thid = *((int*)arg);
    Int2StrHashMap::iterator iter = hashMap.getValue(thid);
    if (iter) {
        printf("thread[%d]$ get keyValue[%d->%s]\n", thid, iter->first, iter->second.c_str());
    } else {
        printf("thread[%d]$ key do not exist\n", thid);
    }

    hashMap.remove(thid);
    printf("thread[%d]$ hashMap after remove key[%d]: ", thid, thid);
    hashMap.for_each(print);
    printf("\nthread[%d]$ thread exit\n", thid);
    return NULL;
}

int main(int argc, const char *argv[])
{
    printf("thread[m]$ test begin....\n");
    int THREAD_NUM = 4;
    if (argc == 2) {
        int num = atoi(argv[1]);
        THREAD_NUM = num > 0 ? num : 4;
    }
    for (int i = 0; i < THREAD_NUM; ++i) {
        hashMap.addOrUpdate(i, "j");
    }
    printf("thread[m]$ original elements in hashMap: ");
    hashMap.for_each(print);
    printf("\n");

    int staticKey = 0;
    Int2StrHashMap::iterator iter = hashMap.getValue(staticKey);
    printf("thread[m]$ get iterator of key[%d] before it removed by other thread\n", staticKey);

    pthread_t thid[THREAD_NUM];
    int localThid[THREAD_NUM];
    for (int i = 0; i < THREAD_NUM; ++i) {
        localThid[i] = i;
        pthread_create(&thid[i], NULL, threadFunc, (void*)&localThid[i]);
    }
    for (int i = 0; i < THREAD_NUM; ++i) {
        pthread_join(thid[i], NULL);
    }

    printf("thread[m]$ node of keyValue[%d->%s] removed by other thread, but still accessable in main thread\n", staticKey, iter->second.c_str());
    printf("thread[m]$ test done\n");
    return 0;
}
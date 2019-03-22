/*
* @Author:             darkbao
* @Date:               2019-03-21 22:54:10
* @Last Modified by:   darkbao
* @Email:              freecare_j@163.com
*/
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include "ThreadSafeList.hpp"

using namespace std;

class element
{
public:
    element(int idx = -1, const char* data = NULL): m_index(idx)
    { 
        if (data) {
            memcpy(m_data, data, 28);
        } else {
            memset(m_data, 0, 28);
        }
    }
    void print() const
    {
        printf("index[%d], data[%s]\n", m_index, m_data);
    }
    inline int getIndex() const
    {
        return m_index;
    }
private:
    int  m_index;
    char m_data[28];
};

mj::ThreadSafeList<element> elist;

void* threadFunc(void* arg)
{
    int thid = *((int*)arg);
    printf("==>thread[%d] begin\n", thid);
    auto pElement = elist.find_first_if([thid](const element& em){ return em.getIndex() == thid; });
    if (pElement) {
        printf("==>thread[%d] get element: ", thid);
        pElement->print();
    } else {
        printf("==>thread[%d] can't find element\n", thid);
    }

    printf("==>thread[%d] after remove element:\n", thid);
    elist.remove_if([thid](const element& em){ return em.getIndex() == thid; });
    elist.for_each([](const element& em){ em.print(); });
    printf("==>thread[%d] exit\n\n", thid);
    return NULL;
}

int main(int argc, const char *argv[])
{
    printf("test begin....\n");
    const int THREAD_NUM = 16;
    for (int i = 0; i < THREAD_NUM; ++i) {
        elist.push_front(element(i, "zmj"));
    }
    printf("original list elements:\n");
    elist.for_each([](const element& em){ em.print(); });
    printf("\n");

    pthread_t thid[THREAD_NUM];
    int localThid[THREAD_NUM];
    for (int i = 0; i < THREAD_NUM; ++i) {
        localThid[i] = i;
        pthread_create(&thid[i], NULL, threadFunc, (void*)&localThid[i]);
    }
    for (int i = 0; i < THREAD_NUM; ++i) {
        pthread_join(thid[i], NULL);
    }
    printf("test done\n");
    return 0;
}
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
        printf("element[%d], data[%s]\n", m_index, m_data);
    }
    bool isIndexOdd() const
    {
        return (m_index & 0x1);
    }
private:
    int  m_index;
    char m_data[28];
};

int main(int argc, const char *argv[])
{
    printf("test begin....\n");
    mj::ThreadSafeList<element> elist;
    int i = 0;
    elist.push_front(element(i++, "zmj"));
    elist.push_front(element(i++, "zoe"));
    printf("==>original list elements:\n");
    elist.for_each([](const element& em){ em.print(); });
    
    auto pElement = elist.find_first_if([](const element& em){ return em.isIndexOdd(); });
    if (pElement) {
        printf("==>first element with odd index:\n");
        pElement->print();
    } else {
        printf("can't find element with odd index\n");
    }

    printf("==>after remove all element with odd index:\n");
    elist.remove_if([](const element& em){ return em.isIndexOdd(); });
    elist.for_each([](const element& em){ em.print(); });

    printf("done\n");
    return 0;
}
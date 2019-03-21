/*
* @Author:             darkbao
* @Date:               2019-03-21 18:52:10
* @Last Modified by:   darkbao
* @Email:              freecare_j@163.com
*/
#ifndef __MJ_THREAD_SAFE_LIST_HPP__
#define __MJ_THREAD_SAFE_LIST_HPP__
#include <mutex>
#include <utility>

namespace mj
{

template<typename T>
class ThreadSafeList
{
public:
    ThreadSafeList() { }
    ~ThreadSafeList() { remove_if( [](const node&){return true;} ); }
    ThreadSafeList(const ThreadSafeList&) = delete;
    ThreadSafeList& operator=(const ThreadSafeList&) = delete;

    void push_front(const T& value)
    {
        node* pNewNode = new node(value);
        std::lock_guard<std::mutex> lock(m_head.mtx);
        pNewNode->pNext = m_head.pNext;
        m_head.pNext = pNewNode;
    }

    template<typename Function>
    void for_each(Function func)
    {
        node* pCurNode = &m_head;
        std::unique_lock<std::mutex> lock(m_head.mtx);
        while (node* pNextNode = pCurNode->pNext) {
            std::unique_lock<std::mutex> nextLock(pNextNode->mtx);
            lock.unlock();
            func(*(pNextNode->pData));
            pCurNode = pNextNode;
            lock = std::move(nextLock);
        }
    }

    template<typename Predicate>
    const T* find_first_if(Predicate pred)
    {
        node* pCurNode = &m_head;
        std::unique_lock<std::mutex> lock(m_head.mtx);
        while (node* pNextNode = pCurNode->pNext) {
            std::unique_lock<std::mutex> nextLock(pNextNode->mtx);
            lock.unlock();
            if (pred(*(pNextNode->pData)))
                return pNextNode->pData;
            pCurNode = pNextNode;
            lock = std::move(nextLock);
        }
        return NULL;
    }

    template<typename Predicate>
    void remove_if(Predicate pred)
    {
        node* pCurNode = &m_head;
        std::unique_lock<std::mutex> lock(m_head.mtx);
        while (node* pNextNode = pCurNode->pNext) {
            std::unique_lock<std::mutex> nextLock(pNextNode->mtx);
            if (pred(*(pNextNode->pData))) {
                pCurNode->pNext = pNextNode->pNext;
                nextLock.unlock();
                delete pNextNode;
            } else {
                lock.unlock();
                pCurNode = pNextNode;
                lock = std::move(nextLock);
            }
        }
    }

private:
    struct node
    {
        std::mutex mtx;
        node* pNext;
        T*    pData;

        node(): pNext(NULL), pData(NULL) { }
        ~node() { if (pData) delete pData; }
        node(const T& value): pNext(NULL), pData(new T(value)) { }
    };
    node m_head;    
};

}
#endif
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
#include <memory>

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
#ifdef __DEBUG_NODE__
        printf("**DEBUG**$ add new node[0x%p]\n", pNewNode);
#endif
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
    std::shared_ptr<T> find_first_if(Predicate pred)
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
        return std::shared_ptr<T>();
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
#ifdef __DEBUG_NODE__
                 printf("**DEBUG**$ delete node[0x%p]\n", pNextNode);
#endif
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
        std::shared_ptr<T> pData;

        node(): pNext(NULL) { }
#ifdef __DEBUG_NODE__
        node(const T& value) : pNext(NULL), 
             pData( new T(value), 
                    [](T* p){ delete p; printf("**DEBUG**$ shared_ptr: delete ptr[0x%p]\n", p);}, 
                    std::allocator<T>() )
        { }
#else
        node(const T& value) : pNext(NULL), pData(new T(value)){ }
#endif
    };
    node m_head;    
};

}
#endif
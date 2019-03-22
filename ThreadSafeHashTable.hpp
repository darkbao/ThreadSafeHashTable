/*
* @Author:             darkbao
* @Date:               2019-03-21 18:52:01
* @Last Modified by:   darkbao
* @Email:              freecare_j@163.com
*/
#ifndef __MJ_THREAD_SAFE_HASH_TABLE_HPP__
#define __MJ_THREAD_SAFE_HASH_TABLE_HPP__
#include <vector>
#include <utility>
#include <functional>
#include "ThreadSafeList.hpp"

namespace mj
{

template<typename Key, typename Value, typename Hash = std::hash<Key>>
class ThreadSafeHashTable
{
private:
    // bucket type inside
    class Bucket
    {
    private:
        typedef std::pair<Key, Value> BucketElement;
        typedef ThreadSafeList<BucketElement> BucketData;
        BucketData m_data;
    public:
        std::shared_ptr<BucketElement> getValue(const Key& key)
        {
            return m_data.find_first_if([&key](const BucketElement& em){ return (key == em.first); });
        }
        void addOrUpdateElement(const Key& key, const Value& value)
        {
            auto pElement = m_data.find_first_if([&key](const BucketElement& em){ return (key == em.first); });
            if (pElement) {
                pElement->second = value;
            } else {
                m_data.push_front(BucketElement(key, value));
            }
        }
        void removeElement(const Key& key)
        {
            m_data.remove_if([&key](const BucketElement& em){ return (key == em.first);});
        }

        template<typename Function>
        void for_each(Function func)
        {
            m_data.for_each(func);
        }
    };
    
    // private member of hash table
    Bucket& getBucket(const Key& key) const
    {
        std::size_t index = m_hashFunc(key) % m_bucketNum;
        return m_pBuckets[index];
    }
    unsigned    m_bucketNum;
    Bucket*     m_pBuckets;
    Hash        m_hashFunc;

public:
    typedef std::shared_ptr<std::pair<Key, Value>> iterator;
    ThreadSafeHashTable(unsigned bucket_num = 19, const Hash& hasher = Hash())
                        : m_bucketNum(bucket_num), m_hashFunc(hasher)
    {
        m_pBuckets = new Bucket[m_bucketNum]();
    }

    ~ThreadSafeHashTable()
    {
        if (m_pBuckets) delete[] m_pBuckets;
    }

    ThreadSafeHashTable(const ThreadSafeHashTable&) = delete;
    ThreadSafeHashTable& operator=(const ThreadSafeHashTable&) = delete;

    iterator getValue(const Key& key)
    {
        return getBucket(key).getValue(key);
    }

    void addOrUpdate(const Key& key, const Value& value)
    {
        getBucket(key).addOrUpdateElement(key, value);
    }

    void remove(const Key& key)
    {
        getBucket(key).removeElement(key);
    }

    template<typename Function>
    void for_each(Function func)
    {
        for (unsigned i = 0; i < m_bucketNum; ++i) {
            m_pBuckets[i].for_each(func);
        }
    }
};

}

#endif
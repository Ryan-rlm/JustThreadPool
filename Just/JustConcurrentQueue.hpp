
#ifndef __JUSTCONCURRENTQUEUE_H__
#define __JUSTCONCURRENTQUEUE_H__

#include <memory>
#include <atomic>

#include "JustConfig.h"

JUST_NSP_START

template<typename T>
class ConcurrentQueue final
{
    struct Node
    {
        using Ptr = Node*;
        using SPtr = std::shared_ptr<Node>;
        using AtmPtr = std::atomic<Ptr>;

        T _val;

        AtmPtr _next { nullptr };
    };

    private:
        std::atomic_uint32_t _size;

        typename Node::AtmPtr _first;
        typename Node::AtmPtr _last;
        Node _head;

    public:
        ConcurrentQueue()
            : _size { 0 }
            , _first { nullptr }
            , _last { nullptr }
            , _head { }
        {
            std::atomic_init<typename Node::Ptr>(&_first, &_head);
            std::atomic_init<typename Node::Ptr>(&_last, &_head);
        }

        ~ConcurrentQueue()
        {
            clear();
        }

        ConcurrentQueue(ConcurrentQueue&&) = delete;
        ConcurrentQueue(const ConcurrentQueue&) = delete;
        ConcurrentQueue& operator=(const ConcurrentQueue&) = delete;
        ConcurrentQueue& operator=(ConcurrentQueue&&) = delete;

        // bool try_push(const T& v);
        // bool try_push(T&& v);
        /**
         * @brief 入队
         *
         * @param v
         */
        void push(const T& v)
        {
            typename Node::Ptr tmp_node = new Node;
            tmp_node->_val = v;
            tmp_node->_next = nullptr;
            typename Node::Ptr last_node = nullptr;

            do
            {
                last_node = _last;
            } while (!(_last.compare_exchange_weak(last_node, tmp_node)));

            last_node->_next = tmp_node;
            ++_size;
        }
        // void push(T&&);

        // bool wait_pop(T&, size_t ms = 1000);
        // bool try_pop(T&);
        bool pop(T& v)
        {
            typename Node::Ptr first_node = nullptr;

            do
            {
                first_node = _first;
                if (nullptr == first_node->_next)
                {
                    return false;
                }

            } while (_first.compare_exchange_weak(first_node, first_node->_next) != true);

            --_size;
            v = std::move(first_node->_val);
            delete first_node;

            return true;
        }

        bool empty() const noexcept
        {
            return true;
        }

        size_t size() const noexcept
        {
            return 0;
        }

        void clear() noexcept
        {

        }
};

JUST_NSP_END

#endif // __JUSTCONCURRENTQUEUE_H__

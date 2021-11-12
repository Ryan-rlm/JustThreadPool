
#ifndef __JUSTCONCURRENTQUEUE_H__
#define __JUSTCONCURRENTQUEUE_H__

#include <algorithm>
#include <iterator>
#include <memory>
#include <atomic>

#include "JustConfig.h"

JUST_NSP_START

template<typename T>
class ConcurrentQueue final
{
    struct Node
    {
        using SPtr = std::shared_ptr<Node>;

        T _val;

        std::shared_ptr<Node> _next = nullptr;
    };

    private:
        std::atomic_uint32_t _size;

        typename Node::SPtr _first;
        typename Node::SPtr _last;

    public:
        ConcurrentQueue()
            : _size { 0 }
            , _first { std::make_shared<Node>() }
            , _last { _first }
        {
        }

        ~ConcurrentQueue()
        {
            stop_and_clear();
        }

        ConcurrentQueue(ConcurrentQueue&&) = delete;
        ConcurrentQueue(const ConcurrentQueue&) = delete;
        ConcurrentQueue& operator=(const ConcurrentQueue&) = delete;
        ConcurrentQueue& operator=(ConcurrentQueue&&) = delete;

        bool is_lock_free()
        {
            return std::atomic_is_lock_free(&_first) && std::atomic_is_lock_free(&_last) && std::atomic_is_lock_free(&_size);
        }

        /**
         * @brief 入队
         *
         * @param v
         */
        bool push(const T& v)
        {
            return push(T(v));
        }

        bool push(T&& v)
        {
            typename Node::SPtr last_node = std::atomic_load(&_last);
            if (nullptr == last_node)
                return false;

            typename Node::SPtr v_node = std::make_shared<Node>();
            v_node->_val = std::move(v);

            do
            {
                last_node = std::atomic_load(&_last);
                if (nullptr == last_node)
                    return false;
            } while (!(std::atomic_compare_exchange_strong(&_last, &last_node, v_node)));

            std::atomic_store(&(last_node->_next), v_node);
            ++_size;

            return true;
        }

        bool wait_pop(T&, size_t ms = 1000)
        {
            return false;
        }

        bool pop(T& v)
        {
            typename Node::SPtr first_node = nullptr;
            typename Node::SPtr first_node_next = nullptr;

            do
            {
                first_node = std::atomic_load(&_first);
                first_node_next = std::atomic_load(&(first_node->_next));
                if (nullptr == first_node
                    || nullptr == first_node_next)
                {
                    return false;
                }

            } while (!(std::atomic_compare_exchange_strong(&_first, &first_node, first_node_next)));

            --_size;
            v = std::move(first_node_next->_val);

            return true;
        }

        bool empty() const noexcept
        {
            return 0 == _size;
        }

        size_t size() const noexcept
        {
            return _size;
        }

        void clear() noexcept
        {
            typename Node::SPtr first_node = std::atomic_load(&_first);
            typename Node::SPtr last_node = stop_push();
            typename Node::SPtr null_node = nullptr;

            if (nullptr == first_node
                || nullptr == last_node)
                return;

            do {
                first_node = std::atomic_load(&_first);
                if (nullptr == first_node)
                    return;
            }while (!(std::atomic_compare_exchange_strong(&_first, &first_node, last_node)));

            std::atomic_store(&_last, last_node);
        }

    private:

        void stop_and_clear() noexcept
        {
            typename Node::SPtr last_node = stop_push();
            typename Node::SPtr first_node = stop_pop();
            //typename Node::SPtr first_node_next = std::atomic_load(&(first_node->_next));

            while (first_node)
            {
                first_node = first_node->_next;
            }
        }

        typename Node::SPtr stop_pop() noexcept
        {
            typename Node::SPtr first_node = std::atomic_load(&_first);
            typename Node::SPtr null_node = nullptr;
            if (nullptr == first_node)
                return nullptr;

            do
            {
                first_node = std::atomic_load(&_first);
            } while (!(std::atomic_compare_exchange_strong(&_first, &first_node, null_node)));

            return first_node;
        }

        typename Node::SPtr stop_push() noexcept
        {
            typename Node::SPtr last_node = std::atomic_load(&_last);
            typename Node::SPtr null_node = nullptr;
            if (nullptr == last_node)
                return nullptr;

            do
            {
                last_node = std::atomic_load(&_last);
            } while (!(std::atomic_compare_exchange_strong(&_last, &last_node, null_node)));

            return last_node;
        }
};

JUST_NSP_END

#endif // __JUSTCONCURRENTQUEUE_H__

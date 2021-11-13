
#ifndef __JUSTCONCURRENTQUEUE_H__
#define __JUSTCONCURRENTQUEUE_H__

#include <memory>
#include <atomic>


namespace Just{

template<typename T>
class ConcurrentQueue final
{
    struct Node
    {
        using SPtr = std::shared_ptr<Node>;

        T _val;
        SPtr _next = nullptr;
    };

    private:
        mutable std::atomic_uint32_t _size;

        mutable std::atomic_bool _bstop_pop;
        typename Node::SPtr _first;
        mutable std::atomic_bool _bstop_push;
        typename Node::SPtr _last;

    public:
        ConcurrentQueue()
            : _size { 0 }
            , _bstop_pop { false }
            , _first { std::make_shared<Node>() }
            , _bstop_push { false }
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
            return (std::atomic_is_lock_free(&_size)
                    && std::atomic_is_lock_free(&_bstop_pop)
                    && std::atomic_is_lock_free(&_first)
                    && std::atomic_is_lock_free(&_bstop_push)
                    && std::atomic_is_lock_free(&_last));
        }

        bool try_push(const T& v)
        {
            return push(T(v));
        }

        bool try_push(T&& v)
        {
            if (_bstop_push)
                return false;

            typename Node::SPtr null_node = nullptr;
            typename Node::SPtr last_node = nullptr;
            typename Node::SPtr v_node = std::make_shared<Node>();
            v_node->_val = std::move(v);
            v_node->_next = nullptr;

            last_node = std::atomic_exchange(&_last, v_node);
            std::atomic_store(&(last_node->_next), v_node);
            ++_size;

            return true;
        }

        bool try_pop(T& v)
        {
            if (_bstop_pop)
                return false;
            typename Node::SPtr first_node = nullptr;
            typename Node::SPtr first_node_next = nullptr;

            do
            {
                first_node = std::atomic_load(&_first);
                first_node_next = std::atomic_load(&(first_node->_next));
                if (nullptr == first_node_next)
                    return false;
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

        void stop_push() noexcept
        {
            _bstop_push = true;
        }

        void start_push() noexcept
        {
            _bstop_push = false;
        }

        void stop_pop() noexcept
        {
            _bstop_pop = true;
        }

        void start_pop() noexcept
        {
            _bstop_pop = false;
        }

        void clear() noexcept
        {
            stop_and_clear();
            start();
        }

        void stop_and_clear() noexcept
        {
            stop_push();
            stop_pop();
            typename Node::SPtr first_node = std::atomic_exchange(&_first, std::atomic_load(&_last));
            _size = 0;

            while (first_node)
                first_node = first_node->_next;
        }

        void start() noexcept
        {
            start_pop();
            start_push();
        }
};

}

#endif // __JUSTCONCURRENTQUEUE_H__

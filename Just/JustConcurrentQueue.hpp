
#ifndef __JUSTCONCURRENTQUEUE_H__
#define __JUSTCONCURRENTQUEUE_H__

#include <memory>
#include <atomic>


namespace Just{


template<typename T>
class ConcurrentQueue final
{
    public:
        struct Node
        {
            using Ptr = Node*;
            using AtomicPtr = std::atomic<Node*>;

            T _val;
            AtomicPtr _next;
        };

    private:
        std::atomic_uint32_t _size;

        std::atomic_bool _bstop_pop;
        typename Node::AtomicPtr _first;
        std::atomic_bool _bstop_push;
        typename Node::AtomicPtr _last;

    public:
        ConcurrentQueue()
            : _size { 0 }
            , _bstop_pop { false }
            , _first { nullptr }
            , _bstop_push { false }
            , _last { nullptr }
        {
            typename Node::Ptr ptr = new Node;
            _first.store(ptr, std::memory_order_relaxed);
            _last.store(ptr, std::memory_order_relaxed);
        }

        ~ConcurrentQueue()
        {
            stop_and_clear();
            delete _last.load(std::memory_order_relaxed);
        }

        ConcurrentQueue(ConcurrentQueue&&) = delete;
        ConcurrentQueue(const ConcurrentQueue&) = delete;
        ConcurrentQueue& operator=(ConcurrentQueue&&) = delete;
        ConcurrentQueue& operator=(const ConcurrentQueue&) = delete;

        bool is_lock_free()
        {
            return (std::atomic_is_lock_free(&_size)
                    && std::atomic_is_lock_free(&_bstop_pop)
                    && std::atomic_is_lock_free(&_first)
                    && std::atomic_is_lock_free(&_bstop_push)
                    && std::atomic_is_lock_free(&_last));
        }

        bool push(const T& v)
        {
            return push(T(v));
        }

        bool push(T&& v)
        {
            if (_bstop_push)
                return false;

            typename Node::Ptr last_node = nullptr;
            typename Node::Ptr v_node = new Node;

            v_node->_val = std::move(v);
            v_node->_next.store(nullptr, std::memory_order_relaxed);

            last_node = _last.exchange(v_node, std::memory_order_relaxed);
            last_node->_next.store(v_node, std::memory_order_relaxed);
            ++_size;

            return true;
        }

        bool pop(T& v)
        {
            if (_bstop_pop)
                return false;
            typename Node::Ptr first_node = _first.load(std::memory_order_relaxed);
            typename Node::Ptr first_node_next = nullptr;

            do
            {
                first_node_next = first_node->_next.load(std::memory_order_relaxed);
                if (nullptr == first_node_next)
                    return false;
            } while (!(_first.compare_exchange_strong(first_node, first_node_next, std::memory_order_release, std::memory_order_relaxed)));

            --_size;
            v = std::move(first_node_next->_val);
            delete first_node;

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
            typename Node::Ptr last_node = _last.load(std::memory_order_relaxed);
            typename Node::Ptr first_node = _first.exchange(last_node, std::memory_order_relaxed);
            typename Node::Ptr del_node = nullptr;
            _size = 0;
            while (first_node && first_node != last_node) {
                del_node = first_node;
                first_node = first_node->_next.load(std::memory_order_relaxed);
                delete del_node;
            };
        }

        void start() noexcept
        {
            start_pop();
            start_push();
        }
};


}

#endif // __JUSTCONCURRENTQUEUE_H__

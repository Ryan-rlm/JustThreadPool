
#ifndef __JUSTCONCURRENTQUEUE_H__
#define __JUSTCONCURRENTQUEUE_H__

#include <cstddef>
#include <new>
#include <memory>
#include <atomic>


namespace Just{

template<typename T>
struct TheNode
{
    using Ptr = TheNode*;
    using AtomicPtr = std::atomic<TheNode*>;

    T _val;
    AtomicPtr _next;
};

template<typename T, typename Allocator = std::allocator<TheNode<T>>>
class ConcurrentQueue final
{
    public:
        using Node = TheNode<T>;

    private:
        std::atomic<int32_t> _size;

        typename Node::AtomicPtr _del;
        typename Node::AtomicPtr _first;
        typename Node::AtomicPtr _last;
        Allocator allocator; 

        typename Node::Ptr get_new_node()
        {
            typename Node::Ptr new_node = get_del_node();

            if (!new_node) {
                new_node = allocator.allocate(1);
            }
            new_node->_next.store(nullptr, std::memory_order_relaxed);
            
            return new_node;
        }

        typename Node::Ptr get_del_node()
        {
            typename Node::Ptr del_node = _del.load(std::memory_order_relaxed);
            typename Node::Ptr del_node_next = del_node->_next.load(std::memory_order_relaxed);
            typename Node::Ptr first_node = _first.load(std::memory_order_relaxed);

            if (del_node != first_node
                && _del.compare_exchange_weak(del_node, del_node_next, std::memory_order_relaxed, std::memory_order_relaxed)) {
                return del_node;
            }
            
            return nullptr;
        }

    public:
        ConcurrentQueue()
            : _size { 0 }
            , _del { nullptr }
            , _first { nullptr }
            , _last { nullptr }
        {
            typename Node::Ptr ptr = new Node;
            _del.store(ptr, std::memory_order_relaxed);
            _first.store(ptr, std::memory_order_relaxed);
            _last.store(ptr, std::memory_order_relaxed);
        }

        /**
         * @brief Destroy the Concurrent Queue object, 需要停止所有 push pop
         *
         */
        ~ConcurrentQueue()
        {
            clear();
            typename Node::Ptr del_node = _del.load(std::memory_order_relaxed);
            typename Node::Ptr del_node_next = nullptr;
            while (del_node) {
                del_node_next = del_node->_next.load(std::memory_order_relaxed);
                allocator.deallocate(del_node, 1);
                del_node = del_node_next;
            };
        }

        ConcurrentQueue(ConcurrentQueue&&) = delete;
        ConcurrentQueue(const ConcurrentQueue&) = delete;
        ConcurrentQueue& operator=(ConcurrentQueue&&) = delete;
        ConcurrentQueue& operator=(const ConcurrentQueue&) = delete;

        bool is_lock_free()
        {
            return (std::atomic_is_lock_free(&_size)
                    && std::atomic_is_lock_free(&_del)
                    && std::atomic_is_lock_free(&_first)
                    && std::atomic_is_lock_free(&_last));
        }

        bool push(const T& v)
        {
            return push(T(v));
        }

        bool push(T&& v)
        {
            typename Node::Ptr last_node = nullptr;
            typename Node::Ptr v_node = get_new_node();
            if (nullptr == v_node)
                return false;

            v_node->_val = std::move(v);

            last_node = _last.exchange(v_node, std::memory_order_relaxed);
            last_node->_next.store(v_node, std::memory_order_relaxed);
            _size.fetch_add(1, std::memory_order_release);

            return true;
        }

        bool pop(T& v)
        {
            if (empty())
                return false;
            typename Node::Ptr first_node = _first.load(std::memory_order_relaxed);
            typename Node::Ptr first_node_next = nullptr;

            do
            {
                first_node_next = first_node->_next.load(std::memory_order_relaxed);
                if (nullptr == first_node_next)
                    return false;
            } while (!(_first.compare_exchange_weak(first_node, first_node_next, std::memory_order_acquire, std::memory_order_relaxed)));

            _size.fetch_sub(1, std::memory_order_release);
            v = std::move(first_node_next->_val);
            allocator.deallocate(get_del_node(), 1);

            return true;
        }

        bool empty() const noexcept
        {
            return size() <= 0;
        }

        int32_t size() const noexcept
        {
            return _size.load(std::memory_order_relaxed);
        }

        /**
         * @brief 强行将头指针指向尾指针
         *
         */
        void clear() noexcept
        {
            typename Node::Ptr const last_node = _last.load(std::memory_order_relaxed);
            typename Node::Ptr first_node = _first.exchange(last_node, std::memory_order_acquire);
            int32_t clear_size = _size.exchange(0, std::memory_order_release);
        }
};


}

#endif // __JUSTCONCURRENTQUEUE_H__

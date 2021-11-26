
#ifndef __JUSTCONCURRENTQUEUE_H__
#define __JUSTCONCURRENTQUEUE_H__

#include <cstddef>
#include <new>
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
        std::atomic<int32_t> _size;

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

        /**
         * @brief Destroy the Concurrent Queue object, 需要停止所有 push, 尽量停止 pop
         *
         */
        ~ConcurrentQueue()
        {
            clear();
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
            if (is_stop_push())
                return false;

            typename Node::Ptr last_node = nullptr;
            typename Node::Ptr v_node = new(std::nothrow) Node;
            if (nullptr == v_node)
                return false;

            v_node->_val = std::move(v);
            v_node->_next.store(nullptr, std::memory_order_relaxed);

            _size.fetch_add(1, std::memory_order_acquire);
            last_node = _last.exchange(v_node, std::memory_order_relaxed);
            last_node->_next.store(v_node, std::memory_order_release);

            return true;
        }

        bool pop(T& v)
        {
            if (is_stop_pop())
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
            delete first_node;

            return true;
        }

        bool empty() const noexcept
        {
            return 0 == size();
        }

        size_t size() const noexcept
        {
            return _size.load(std::memory_order_relaxed);
        }

        /**
         * @brief 停止push，获取之前的状态
         *
         * @return true 之前已经停止push
         * @return false 之前正常push
         */
        bool stop_push() noexcept
        {
            return _bstop_push.exchange(true, std::memory_order_relaxed);
        }

        /**
         * @brief 开始push，获取之前的值
         *
         * @return true 之前已经停止push
         * @return false 之前正常push
         */
        bool start_push() noexcept
        {
            return _bstop_push.exchange(false, std::memory_order_relaxed);
        }

        /**
         * @brief 获取push状态
         *
         * @return true 停止push
         * @return false 正常push
         */
        bool is_stop_push() const noexcept
        {
            return _bstop_push.load(std::memory_order_relaxed);
        }

        /**
         * @brief 停止pop，获取之前状态
         *
         * @return true 之前已经停止pop
         * @return false 之前正常pop
         */
        bool stop_pop() noexcept
        {
            return _bstop_pop.exchange(true, std::memory_order_relaxed);
        }

        /**
         * @brief 开始pop，获取之前状态
         *
         * @return true 之前已经停止pop
         * @return false 之前正常pop
         */
        bool start_pop() noexcept
        {
            return _bstop_pop.exchange(false, std::memory_order_relaxed);
        }

        /**
         * @brief 获取pop状态
         *
         * @return true 停止pop
         * @return false 正常pop
         */
        bool is_stop_pop() const noexcept
        {
            return _bstop_pop.load(std::memory_order_relaxed);
        }

        /**
         * @brief 强行将头指针指向尾指针，中间节点析构，不需要停止 push 或 pop
         *
         */
        void clear() noexcept
        {
            int32_t clear_size = _size.exchange(0, std::memory_order_relaxed);
            typename Node::Ptr const last_node = _last.load(std::memory_order_relaxed);
            typename Node::Ptr first_node = _first.exchange(last_node, std::memory_order_relaxed);
            typename Node::Ptr del_node = nullptr;

            while (first_node != last_node) {
                del_node = first_node;
                do {
                    first_node = del_node->_next.load(std::memory_order_relaxed);
                }while (nullptr == first_node);
                delete del_node;
                --clear_size;
            };

            _size.fetch_add(clear_size, std::memory_order_relaxed);
        }
};


}

#endif // __JUSTCONCURRENTQUEUE_H__

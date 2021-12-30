
#ifndef __JUSTCQ_H__
#define __JUSTCQ_H__

#include <atomic>
#include <utility>

namespace Just{

constexpr const uint32_t BLOCK_SIZE = (1 << 10);

enum class PStat : uint16_t{
    OK,
    FULL,
    OUT_MEM,
};

template<typename T>
struct Block
{

    std::atomic_uint32_t _first;
    std::atomic_uint32_t _last;
    
    Block* _next;

    std::atomic_bool _sflag[BLOCK_SIZE];
    T _data[BLOCK_SIZE];

    Block():
        _first(0),
        _last(0),
        _next(nullptr)
    {}

    Block(Block& other) = delete;
    Block(Block&& other) = delete;

    void clear()
    {
        _last.store(0, std::memory_order_relaxed);
        _first.store(0, std::memory_order_relaxed);
    }

    PStat push(T&& item)
    {
        uint32_t last = _last.fetch_add(1, std::memory_order_relaxed);
        
        if (last == BLOCK_SIZE)
            return PStat::FULL;
        else if (last > BLOCK_SIZE)
            return PStat::OUT_MEM;

        new (_data + last) T(std::move(item));
        _sflag[last].store(true, std::memory_order_release);

        return PStat::OK;
    }

    bool pop(T& item)
    {
        uint32_t first = 0;
        uint32_t last = 0;
        
        first = _first.load(std::memory_order_relaxed);
        do {
            last = _last.load(std::memory_order_relaxed);
            if (first >= last)
                return false;
            if (first >= BLOCK_SIZE)
            {
                clear();
                return false;
            }
            if (!_sflag[first].load(std::memory_order_acquire))
                return false;
        }while (!_first.compare_exchange_weak(first, first+1, std::memory_order_relaxed, std::memory_order_relaxed));

        item = std::move(_data[first]);
        _sflag[first].store(false, std::memory_order_relaxed);

        return true;
    }

};

template<typename T>
class ConcurrentQueue2
{
    using Block_t = Block<T>;
    Block_t* _tie_block;

public:
    class Producer
    {
        std::atomic<Block_t*> _block;
        friend class ConcurrentQueue2;

        public:
            Producer(Block_t* block):
                _block(block)
            {}

            bool push(T& item)
            {
                return push(T(item));
            }

            bool push(T&& item)
            {
                Block_t* block = _block.load(std::memory_order_relaxed);

                while (true) {
                    PStat ps = block->push(std::move(item));

                    switch (ps) {
                        case PStat::OK:
                            goto end_label;
                        case PStat::FULL:
                            block = new_block();
                            break;
                        case PStat::OUT_MEM:
                            block = _block.load(std::memory_order_relaxed);
                            break;
                        default:
                            break;
                    }
                };

                end_label:

                return true;
            }
        
        private:
            Block_t* new_block()
            {
                Block_t* block = _block.load(std::memory_order_relaxed);
                Block_t* next_block = block->_next;

                if (next_block->_last.load(std::memory_order_relaxed) >= BLOCK_SIZE)
                {
                    Block_t* next_block = new Block_t;
                    next_block->_next = block->_next;
                    block->_next = next_block;
                }

                _block.store(next_block, std::memory_order_release);

                return next_block;
            }
    };

    class Customer
    {
        std::atomic<Block_t*> _block;
        friend class ConcurrentQueue2;

        public:
            Customer(Block_t* block):
                _block(block)
            {

            }

            bool pop(T& item)
            {
                Block_t* block = _block.load(std::memory_order_relaxed);

                while (true) {
                    if (!block)
                        return false;
                    if (!block->pop(item))
                        block = next_block();
                };

                return true;
            }

        private:
            Block_t* next_block()
            {
                Block_t* block = _block.load(std::memory_order_relaxed);
                _block.store(block->_next);
                
                return (block == block->_next) ? nullptr : block->_next;
            }
    };

    ConcurrentQueue2():
        _tie_block(new Block_t)
    {
        _tie_block->_next = _tie_block;
    }

    void CreateProducer(Producer& p)
    {
        p._block.store(_tie_block, std::memory_order_relaxed);
    }

    void CreateCustomer(Customer& c)
    {
        c._block.store(_tie_block, std::memory_order_relaxed);
    }

};

}

#endif // __JUSTCQ_H__

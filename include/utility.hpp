#ifndef ELF_UTILITY_HPP
#define ELF_UTILITY_HPP


namespace elf {
    void _warn(const char *file, int line, const char *msg) {
        std::cerr << "Warn at file " << file << ", line " << line << ": " << msg << std::endl;
    }

#define elf_warn(msg) elf::_warn(__FILE__, __LINE__, msg)

    __attribute__((noreturn)) void _abort(const char *file, int line, const char *msg) {
        std::cerr << "Abort at file " << file << ", line " << line << ": " << msg << std::endl;

        abort();
    }

#define elf_abort(msg) elf::_abort(__FILE__, __LINE__, msg)

    __attribute__((noreturn)) void _unreachable(const char *file, int line, const char *msg) {
        std::cerr << "Unreachable at file " << file << ", line " << line << ": " << msg << std::endl;

        abort();
    }

#define elf_unreachable(msg) elf::_unreachable(__FILE__, __LINE__, msg)

#define elf_unused __attribute__((unused))

    using i8 = int8_t;
    using u8 = u_int8_t;
    using i16 = int16_t;
    using u16 = u_int16_t;
    using i32 = int32_t;
    using u32 = u_int32_t;
    using i64 = int64_t;
    using u64 = u_int64_t;
#if defined(__x86_64__)
    using isize = int64_t;
    using usize = u_int64_t;
#else
    using isize = int32_t;
    using usize = u_int32_t;
#endif

    class MappedFileVisitor {
    private:
        void *inner;
        size_t size;

    public:
        explicit MappedFileVisitor(void *inner, size_t size) : inner{inner}, size{size} {}

        bool check_address(u32 offset, size_t len) const {
            return len <= size && offset <= size - len;
        }

        void *trusted_address(u32 offset) const { return static_cast<u8 *>(inner) + offset; }

        void *address(u32 offset, size_t len) const {
            return check_address(offset, len) ? trusted_address(offset) : nullptr;
        }
    };

    template<typename T>
    class ArrayIterator {
    private:
        size_t size;
        void *inner;

    public:
        explicit ArrayIterator(T *inner, size_t size)
                : size{size}, inner{inner} {}

        bool operator!=(const ArrayIterator &other) const { return inner != other.inner; }

        ArrayIterator operator++() {
            inner = static_cast<u8 *>(inner) + size;
            return *this;
        }

        ArrayIterator operator++(int) {
            ArrayIterator ret = *this;
            inner = static_cast<u8 *>(inner) + size;
            return ret;
        }

        T &operator*() const { return *reinterpret_cast<T *>(inner); }

        T *operator->() const { return reinterpret_cast<T *>(inner); }

        T &operator[](size_t index) const {
            u8 *ptr = static_cast<u8 *>(inner) + size * index;
            return *reinterpret_cast<T *>(ptr);
        }
    };
}


#endif //ELF_UTILITY_HPP

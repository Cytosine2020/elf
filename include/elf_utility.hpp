#ifndef ELF_UTILITY_HPP
#define ELF_UTILITY_HPP


#include <iostream>
#include <cstddef>
#include <type_traits>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>


namespace elf {
#define elf_static_inline static inline __attribute__((always_inline))
#define elf_unused __attribute__((unused))
#define elf_no_return __attribute__((noreturn))
#if defined(__DEBUG__)
#define neutron_inline
#else
#define neutron_inline inline __attribute__((__always_inline__))
#endif

    elf_static_inline void _warn(const char *file, int line, const char *msg) {
        std::cerr << "Warn at file " << file << ", line " << line << ": " << msg << std::endl;
    }

#define elf_warn(msg) elf::_warn(__FILE__, __LINE__, msg)

    elf_static_inline elf_no_return void _abort(const char *file, int line, const char *msg) {
        std::cerr << "Abort at file " << file << ", line " << line << ": " << msg << std::endl;

        abort();
    }

#define elf_abort(msg) elf::_abort(__FILE__, __LINE__, msg)

    elf_static_inline elf_no_return void _unreachable(const char *file, int line, const char *msg) {
        std::cerr << "Unreachable at file " << file << ", line " << line << ": " << msg << std::endl;

        abort();
    }

#define elf_unreachable(msg) elf::_unreachable(__FILE__, __LINE__, msg)

#include "recursive_def.hpp"

#define elf_enum_display(Name, base_type, num, ...) \
    enum Name : base_type { \
        _elf_enum_display_attribute_helper_##num(__VA_ARGS__) \
    }; \
    friend std::ostream &operator<<(std::ostream &stream, Name self) { \
        switch (self) { \
            _elf_enum_display_display_helper_##num(__VA_ARGS__) \
            default: \
                stream << '[' << static_cast<base_type>(self) << ']'; \
        } \
        return stream; \
    }

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

    template<typename T, usize end, usize begin>
    struct bits_mask {
    private:
        using RetT = typename std::enable_if<(std::is_unsigned<T>::value && sizeof(T) * 8 >= end &&
                                              end > begin), T>::type;

    public:
        static constexpr RetT val = ((static_cast<T>(1u) << (end - begin)) - static_cast<T>(1u)) << begin;
    };

    template<typename T, usize end, usize begin, isize offset = 0, bool flag = (begin > offset)>
    struct _get_bits;

    template<typename T, usize end, usize begin, isize offset>
    struct _get_bits<T, end, begin, offset, true> {
    public:
        static constexpr T inner(T val) {
            return (val >> (begin - offset)) & bits_mask<T, end - begin, 0>::val << offset;
        }
    };

    template<typename T, usize end, usize begin, isize offset>
    struct _get_bits<T, end, begin, offset, false> {
    public:
        static constexpr T inner(T val) {
            return (val << (offset - begin)) & bits_mask<T, end - begin, 0>::val << offset;
        }
    };

    template<typename T, usize end, usize begin, isize offset = 0>
    constexpr inline T get_bits(T val) {
        static_assert(sizeof(T) * 8 >= end, "end exceed length");
        static_assert(end > begin, "end need to be bigger than start");
        static_assert(sizeof(T) * 8 >= end - begin + offset, "result exceed length");

        return _get_bits<T, end, begin, offset>::inner(val);
    }

    class MappedFileVisitor {
    private:
        int fd;
        void *inner;
        usize size;

        void clear() {
            fd = -1;
            inner = nullptr;
            size = 0;
        }

        void release() {
            munmap(inner, size);
            if (fd != -1) { close(fd); }
            clear();
        }

    public:
        static MappedFileVisitor open_elf(const char *name) {
#if defined(__linux__)
            int fd = open(name, O_RDONLY | F_SHLCK);
#elif defined(__APPLE__)
            int fd = open(name, O_RDONLY | O_SHLOCK);
#else
#error "OS not supported"
#endif
            MappedFileVisitor elf_visitor{};
            elf_visitor.load_file(fd);
            return elf_visitor;
        }

        MappedFileVisitor() : fd{-1}, inner{nullptr}, size{0} {}

        MappedFileVisitor(MappedFileVisitor &&other) noexcept:
                fd{other.fd}, inner{other.inner}, size{other.size} { other.clear(); }

        MappedFileVisitor &operator=(MappedFileVisitor &&other) noexcept {
            if (this != &other) {
                release();

                this->fd = other.fd;
                this->inner = other.inner;
                this->size = other.size;

                other.clear();
            }

            return *this;
        }

        bool load_file(int _fd) {
            release();

            if (_fd == -1) return false;

            fd = _fd;

            struct stat file_stat{};
            if (fstat(fd, &file_stat) != 0) return false;
            size = file_stat.st_size;

            inner = mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0);
            return inner != MAP_FAILED;
        }

        bool check_address(usize offset, usize len) const { return len <= size && offset <= size - len; }

        void *trusted_address(usize offset) const { return static_cast<u8 *>(inner) + offset; }

        void *address(usize offset, usize len) const {
            return check_address(offset, len) ? trusted_address(offset) : nullptr;
        }

        int get_fd() const { return fd; }

        ~MappedFileVisitor() { release(); }
    };

    template<typename T>
    class ArrayIterator {
    private:
        usize size;
        void *inner;

    public:
        ArrayIterator(T *inner, usize size) : size{size}, inner{inner} {}

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

        T &operator[](usize index) const {
            u8 *ptr = static_cast<u8 *>(inner) + size * index;
            return *reinterpret_cast<T *>(ptr);
        }
    };

    u32 elf_hash(const char *name) {
        u32 h = 0;

        while (*name != '\0') {
            h = (h << 4u) + *name++;
            u32 g = h & 0xf0000000;
            if (g != 0) {
                h ^= g >> 24u;
            }
            h &= -g;
        }

        return h;
    }
}


#endif //ELF_UTILITY_HPP

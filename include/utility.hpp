#ifndef ELF_UTILITY_HPP
#define ELF_UTILITY_HPP


namespace elf {
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

    class MappedIOVisitor {
    private:
        void *inner;

    public:
        explicit MappedIOVisitor(void *inner) : inner{inner} {}

        void *address(u32 offset) const { return static_cast<u8 *>(inner) + offset; }
    };
}


#endif //ELF_UTILITY_HPP

#ifndef ELF_PROGRAM_HEADER_HPP
#define ELF_PROGRAM_HEADER_HPP


namespace elf {
    template<typename USizeT>
    class _ProgramHeader;

    template<>
    class _ProgramHeader<u32> {
    public:
        u32 type;
        u32 offset;
        u32 virtual_address;
        u32 physical_address;
        u32 file_size;
        u32 mem_size;
        u32 flags;
        u32 alignment;
    };

    template<>
    class _ProgramHeader<u64> {
    public:
        u32 type;
        u32 flags;
        u64 offset;
        u64 virtual_address;
        u64 physical_address;
        u64 file_size;
        u64 mem_size;
        u64 alignment;
    };

    template<typename USizeT>
    class ProgramHeader : public _ProgramHeader<USizeT> {
    public:
        elf_enum_display(ProgramHeaderType, u32, 8,
                         PROGRAM_NULL, 0,
                         LOADABLE, 1,
                         DYNAMIC_LINK_TABLE, 2,
                         INTERPRETER_PATH_NAME, 3,
                         NOTE, 4,
                         SHARED_LIBRARY, 5,
                         PROGRAM_HEADER_TABLE, 6,
                         THREAD_LOCAL_STORAGE, 7
        );

        template<typename T>
        static T *cast(ProgramHeader *self, MappedFileVisitor &visitor) {
            if (self->type != T::TYPE) return nullptr;
            if (!visitor.check_address(self->offset, self->file_size)) return nullptr;

            return reinterpret_cast<T *>(self);
        }

        static constexpr USizeT EXECUTE = 1;
        static constexpr USizeT WRITE = 2;
        static constexpr USizeT READ = 4;

        /// type: This member tells what kind of segment this array element describes or how to
        /// interpret the array element's information. Type values and their meanings appear
        /// below.
        ///
        /// offset: This member gives the offset from the beginning of the file at which the first byte of
        /// the segment resides.
        ///
        /// virtual_address: This member gives the virtual address at which the first byte of the segment resides
        /// in memory.
        ///
        /// physical_address: On systems for which physical addressing is relevant, this member is reserved for the
        /// segment's physical address. This member requires operating system specific information.
        ///
        /// file_size: This member gives the number of bytes in the file image of the segment; it may be zero.
        ///
        /// mem_size: This member gives the number of bytes in the memory image of the segment; it may be zero.
        ///
        /// flags: This member gives flags relevant to the segment. Defined flag values appear below.
        ///
        /// alignment: Loadable process segments must have congruent values for `virtual_address` and
        /// `physical_address`, modulo the page size. This member gives the value to which the
        /// segments are aligned in memory and in the file. Values 0 and 1 mean that no alignment
        /// is required. Otherwise, `alignment` should be a positive, integral power of 2, and
        /// `address` should equal `offset`, modulo `alignment`.

        ProgramHeaderType get_type() const { return static_cast<ProgramHeaderType>(this->type); }

        bool is_execute() const { return (this->flags & EXECUTE) > 0; }

        bool is_write() const { return (this->flags & WRITE) > 0; }

        bool is_read() const { return (this->flags & READ) > 0; }

        friend std::ostream &operator<<(std::ostream &stream, const ProgramHeader &self) {
            stream << "ELF" << sizeof(USizeT) * 8 << "ProgramHeader {\n";
            stream << "\ttype: " << self.get_type() << ",\n";
            stream << "\toffset: " << self.offset << ",\n";
            stream << "\tvirtual_address: " << self.virtual_address << ",\n";
            stream << "\tphysical_address: " << self.physical_address << ",\n";
            stream << "\tfile_size: " << self.file_size << ",\n";
            stream << "\tmem_size: " << self.mem_size << ",\n";
            stream << "\tflags: " << self.flags << ",\n";
            stream << "\talignment: " << self.alignment << ",\n";
            stream << '}';

            return stream;
        }
    };

    template<typename USizeT>
    class ExecutableHeader : public ProgramHeader<USizeT> {
    public:
        static constexpr u32 TYPE = ProgramHeader<USizeT>::LOADABLE;
    };

    template<typename USizeT>
    class InterPathHeader : public ExecutableHeader<USizeT> {
    public:
        static constexpr u32 TYPE = ProgramHeader<USizeT>::INTERPRETER_PATH_NAME;

        const char *get_path_name(MappedFileVisitor &visitor) {
            auto *str = reinterpret_cast<const char *>(visitor.address(this->offset, this->file_size));
            if (strnlen(str, this->file_size) == this->file_size) return nullptr;
            return str;
        }
    };
}

namespace elf32 {
    using ProgramHeader = elf::ProgramHeader<elf::u32>;
    using ExecutableHeader = elf::ExecutableHeader<elf::u32>;
    using InterPathHeader = elf::InterPathHeader<elf::u32>;
}

namespace elf64 {
    using ProgramHeader = elf::ProgramHeader<elf::u64>;
    using ExecutableHeader = elf::ExecutableHeader<elf::u64>;
    using InterPathHeader = elf::InterPathHeader<elf::u64>;
}


#endif //ELF_PROGRAM_HEADER_HPP

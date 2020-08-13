#ifndef ELF_SECTION_HEADER_HPP
#define ELF_SECTION_HEADER_HPP


namespace elf {
    template<typename USizeT>
    class SectionHeader {
    public:
        elf_enum_display(SectionHeaderType, u32, 15,
                         SECTION_NULL, 0,               /// marks an unused section header
                         PROGRAM_BITS, 1,               /// information defined by the program
                         SYMBOL_TABLE, 2,               /// a linker symbol table
                         STRING_TABLE, 3,               /// a string table
                         RELOCATION_ADDEND_TABLE, 4,    /// “Rela” type relocation entries
                         HASH_TABLE, 5,                 /// a symbol hash table
                         DYNAMIC_LINKING_TABLE, 6,      /// dynamic linking tables
                         NOTE, 7,                       /// note information
                         NO_BITS, 8,                    /// uninitialized space; does not occupy any space in the file
                         RELOCATION_TABLE, 9,           /// “Rel” type relocation entries
                         SHARED_LIBRARY, 10,            /// reserved
                         DYNAMIC_SYMBOL_TABLE, 11,      /// a dynamic loader symbol table
                         INITIALIZE_ARRAY, 14,          /// an array of pointers to initialization functions
                         TERMINATION_ARRAY, 15,         /// an array of pointers to termination functions
                         PRE_INITIALIZE_ARRAY, 16       /// an array of pointers to pre-initialization functions
        );

        static constexpr USizeT WRITE = 1;
        static constexpr USizeT ALLOCATE = 2;
        static constexpr USizeT EXECUTABLE = 4;

        template<typename T>
        static T *cast(SectionHeader *self, MappedFileVisitor &visitor) {
            if (self->section_type != T::TYPE) return nullptr;
            if (self->entry_size < T::ENTRY_SIZE) return nullptr;
            if (self->entry_size != 0 && self->size % self->entry_size != 0) return nullptr;
            if (!visitor.check_address(self->offset, self->size)) return nullptr;

            return reinterpret_cast<T *>(self);
        }

        ///	contains the offset, in bytes, to the section name, relative to the start of the section
        /// name string table.
        u32 name;
        /// identifies the section type.
        SectionHeaderType section_type;
        /// identifies the attributes of the section.
        USizeT flags;
        /// contains the virtual address of the beginning of the section in memory. If the section is
        /// not allocated to the memory image of the program, this field should be zero.
        USizeT address;
        /// contains the offset, in bytes, of the beginning of the section contents in the file.
        USizeT offset;
        /// contains the size, in bytes, of the section. Except for ShtNoBits sections, this is the
        /// amount of space occupied in the file.
        USizeT size;
        /// contains the section index of an associated section. This field is used for several
        /// purposes, depending on the type of section, as explained in Table 10.
        u32 link;
        /// contains extra information about the section. This field is used for several purposes,
        /// depending on the type of section, as explained in Table 11.
        u32 info;
        /// contains the required alignment of the section. This field must be a power of two.
        USizeT alignment;
        /// contains the size, in bytes, of each entry, for sections that contain fixed-size entries.
        /// Otherwise, this field contains zero.
        USizeT entry_size;

        bool is_write() const { return (flags & WRITE) > 0; }

        bool is_allocate() const { return (flags & ALLOCATE) > 0; }

        bool is_executable() const { return (flags & EXECUTABLE) > 0; }

        friend std::ostream &operator<<(std::ostream &stream, const SectionHeader &self) {
            stream << "ELF" << sizeof(USizeT) * 8 << "SectionHeader {\n";
            stream << "\tname: " << self.name << ",\n";
            stream << "\tsection_type: " << self.section_type << ",\n";
            stream << "\tflags: " << (self.is_write() ? "W" : "") << (self.is_allocate() ? "A" : "")
                   << (self.is_executable() ? "E" : "") << ",\n";
            stream << "\taddress: " << self.address << ",\n";
            stream << "\toffset: " << self.offset << ",\n";
            stream << "\tsize: " << self.size << ",\n";
            stream << "\tlink: " << self.link << ",\n";
            stream << "\tinfo: " << self.info << ",\n";
            stream << "\talignment: " << self.alignment << ",\n";
            stream << "\tentry_size: " << self.entry_size << ",\n";
            stream << '}';

            return stream;
        }
    };

    template<typename USizeT, typename EntryT>
    class SectionIterable {
    private:
        SectionHeader<USizeT> &section;
        MappedFileVisitor &visitor;

    public:
        SectionIterable(SectionHeader<USizeT> &section, MappedFileVisitor &visitor) :
                section{section}, visitor{visitor} {}

        using Iter = ArrayIterator<EntryT>;

        Iter begin() const {
            void *ptr = visitor.trusted_address(section.offset);
            return Iter{reinterpret_cast<EntryT *>(ptr), section.entry_size};
        }

        Iter end() const {
            void *ptr = visitor.trusted_address(section.offset + section.size);
            return Iter{reinterpret_cast<EntryT *>(ptr), section.entry_size};
        }

        EntryT &operator[](usize index) const {
            if (index * section.entry_size >= section.size) elf_abort("index out of boundary!");
            return begin()[index];
        }
    };

    template<typename USizeT>
    class StringTableHeader : public SectionHeader<USizeT> {
    public:
        class StringTable {
        private:
            StringTableHeader &header;
            MappedFileVisitor &visitor;

        public:
            StringTable(StringTableHeader &header, MappedFileVisitor &visitor) :
                    header{header}, visitor{visitor} {}

            const char *get_str(usize index, const char *no_name = "") const {
                if (index >= header.size) return nullptr;
                if (index == 0) return no_name;
                char *str = reinterpret_cast<char *>(visitor.trusted_address(header.offset + index));
                if (strnlen(str, header.size - index) == (header.size - index)) return nullptr;
                return str;
            }
        };

        static constexpr u32 TYPE = SectionHeader<USizeT>::STRING_TABLE;
        static constexpr usize ENTRY_SIZE = 0;

        using TableT = StringTable;

        TableT get_table(MappedFileVisitor &visitor) { return TableT{*this, visitor}; }
    };

    template<typename USizeT>
    class _SymbolTableEntry;

    template<>
    class _SymbolTableEntry<u32> {
    public:
        u32 name;
        u32 value;
        u32 size;
        u8 info;
        u8 other;
        u16 section_header_index;
    };

    template<>
    class _SymbolTableEntry<u64> {
    public:
        u32 name;
        u8 info;
        u8 other;
        u16 section_header_index;
        u64 value;
        u64 size;
    };

    template<typename USizeT>
    class _SymbolTableHeader : public SectionHeader<USizeT> {
    public:
        elf_enum_display(SymbolBinding, u8, 3,
                         LOCAL, 0,
                         GLOBAL, 1,
                         WEAK, 2
        );

        elf_enum_display(SymbolType, u8, 5,
                         NO_TYPE, 0,
                         OBJECT, 1,
                         FUNCTION, 2,
                         SECTION, 3,
                         FILE, 4
        );

        elf_enum_display(SymbolVisibility, u8, 4,
                         DEFAULT, 0,
                         INTERNAL, 1,
                         HIDDEN, 2,
                         PROTECTED, 3
        );

        class SymbolTableEntry : public _SymbolTableEntry<USizeT> {
        public:
            /// name: contains the offset, in bytes, to the symbol name, relative to the start of the symbol string
            ///     table. If this field contains zero, the symbol has no name.
            ///
            /// info: contains the symbol type and its binding attributes (that is, its scope). The binding attributes
            ///     are contained in the high-order four bits of the eight-bit byte, and the symbol type is contained
            ///     in the low-order four bits. An STT_FILE symbol must have STB_LOCAL binding, its section index
            ///     must be SHN_ABS, and it must precede all other local symbols for the file.
            ///
            /// other: is reserved for future use; must be zero.
            ///
            /// section_header_index: contains the section index of the section in which the symbol is “defined.” For
            ///     undefined symbols, this field contains SHN_UNDEF; for absolute symbols, it contains SHN_ABS; and
            ///     for common symbols, it contains SHN_COMMON.
            ///
            /// value: contains the value of the symbol. This may be an absolute value or a relocatable address.
            ///
            /// size: contains the size associated with the symbol. If a symbol does not have an associated size, or
            ///     the size is unknown, this field contains zero.

            SymbolBinding get_bind() const { return static_cast<SymbolBinding>(get_bits<u8, 8, 4>(this->info)); }

            SymbolType get_type() const { return static_cast<SymbolType>(get_bits<u8, 4, 0>(this->info)); }

            SymbolVisibility get_visibility() const {
                return static_cast<SymbolVisibility>(get_bits<u8, 2, 0>(this->other));
            }

            friend std::ostream &operator<<(std::ostream &stream, const SymbolTableEntry &self) {
                stream << "ELF" << sizeof(USizeT) * 8 << "SymbolTableEntry {\n";
                stream << "\tname: " << self.name << ",\n";
                stream << "\tbind: " << self.get_bind() << ",\n";
                stream << "\ttype: " << self.get_type() << ",\n";
                stream << "\tvisibility: " << self.get_visibility() << ",\n";
                stream << "\tsection_header_index: " << self.section_header_index << ",\n";
                stream << "\tvalue: " << self.value << ",\n";
                stream << "\tsize: " << self.size << ",\n";
                stream << '}';

                return stream;
            }
        };

        static constexpr usize ENTRY_SIZE = sizeof(SymbolTableEntry);

        using TableT = SectionIterable<USizeT, SymbolTableEntry>;

        TableT get_table(MappedFileVisitor &visitor) { return TableT{*this, visitor}; }
    };

    template<typename USizeT>
    class SymbolTableHeader : public _SymbolTableHeader<USizeT> {
    public:
        static constexpr u32 TYPE = SectionHeader<USizeT>::SYMBOL_TABLE;
    };

    template<typename USizeT>
    class DynSymbolTableHeader : public _SymbolTableHeader<USizeT> {
    public:
        static constexpr u32 TYPE = SectionHeader<USizeT>::DYNAMIC_SYMBOL_TABLE;
    };

    template<typename USizeT, typename EntryT>
    class _RelocationTableHeader : public SectionHeader<USizeT> {
    public:
        static constexpr usize ENTRY_SIZE = sizeof(EntryT);

        using TableT = SectionIterable<USizeT, EntryT>;

        TableT get_table(MappedFileVisitor &visitor) { return TableT{*this, visitor}; }
    };

    template<typename USizeT>
    struct RelocationEntry {
        /// offset: indicates the location at which the relocation should be applied. For a
        /// relocatable file, this is the offset, in bytes, from the beginning of the section to
        /// the beginning of the storage unit being relocated. For an executable or shared object,
        /// this is the virtual address of the storage unit being relocated.
        USizeT offset;
        /// info: contains both a symbol table index and a relocation type. The symbol table index
        /// identifies the symbol whose value should be used in the relocation. Relocation types
        /// are processor specific. The symbol table index is obtained by applying the ELF64_R_SYM
        /// macro to this field, and the relocation type is obtained by applying the ELF64_R_TYPE
        /// macro to this field. The ELF64_R_INFO macro combines a symbol table index and a
        /// relocation type to produce a value for this field. These macros are defined as follows:
        USizeT info;

        usize get_symbol();

        usize get_type();
    };

    template<>
    usize RelocationEntry<u32>::get_symbol() { return info >> 8u; }

    template<>
    usize RelocationEntry<u32>::get_type() { return info & 0xffu; }

    template<>
    usize RelocationEntry<u64>::get_symbol() { return info >> 32u; }

    template<>
    usize RelocationEntry<u64>::get_type() { return info & 0xfffffffflu; }

    template<typename USizeT>
    struct RelocationAddendEntry : public RelocationEntry<USizeT> {
        /// specifies a constant addend used to compute the value to be stored in the relocated
        /// field.
        USizeT addend;
    };

    template<typename USizeT>
    class RelocationTableHeader : public _RelocationTableHeader<USizeT, RelocationEntry<USizeT>> {
    public:
        static constexpr u32 TYPE = SectionHeader<USizeT>::RELOCATION_TABLE;
    };

    template<typename USizeT>
    class RelocationTableAddendHeader : public _RelocationTableHeader<USizeT, RelocationAddendEntry<USizeT>> {
    public:
        static constexpr u32 TYPE = SectionHeader<USizeT>::RELOCATION_ADDEND_TABLE;
    };

    template<typename USizeT>
    class DynLinkingTableHeader : public SectionHeader<USizeT> {
    public:
        elf_enum_display(DynLinkingTag, USizeT, 35,
                         DYNAMIC_LINK_NULL, 0,      /// Marks the end of the dynamic array
                         NEEDED, 1,                 /// The string table offset of the name of a needed library.
                         PLT_ENTRY_SIZE, 2,         /// Total size, in bytes, of the relocation entries associated
                                                    /// with the procedure linkage table.
                         PLTGOT, 3,                 /// Contains an address associated with the linkage table. The
                                                    /// specific meaning of this field is processor-dependent.
                         HASH, 4,                   /// Address of the symbol hash table, described below.
                         STRING_TABLE, 5,           /// Address of the dynamic string table.
                         SYMBOL_TABLE, 6,           /// Address of the dynamic symbol table.
                         RELA, 7,                   /// Address of a relocation table.
                         RELA_SIZE, 8,              /// Total size, in bytes, of the relocation relocation table.
                         RELA_ENTRY_SIZE, 9,        /// Size, in bytes, of each relocation relocation entry.
                         STRING_TABLE_SIZE, 10,     /// Total size, in bytes, of the string table.
                         SYMBOL_ENTRY_SIZE, 11,     /// Size, in bytes, of each symbol table entry.
                         INITIALIZE_FUNCTION, 12,   /// Address of the initialization function.
                         TERMINATION_FUNCTION, 13,  /// Address of the termination function.
                         SONAME, 14,                /// The string table offset of the name of this shared object.
                         RPATH, 15,                 /// The string table offset of a shared library search path string.
                         SYMBOLIC, 16,              /// The presence of this dynamic table entry modifies the symbol
                                                    /// resolution algorithm for references within the library. Symbols
                                                    /// defined within the library are used to resolve references
                                                    /// before the dynamic linker searches the usual search path.
                         REL_TABLE, 17,             /// Address of a relocation table.
                         REL_SIZE, 18,              /// Total size, in bytes, of the relocation table.
                         REL_ENTRY_SIZE, 19,        /// Size, in bytes, of each relocation entry.
                         PLT_REL, 20,               /// Type of relocation entry used for the procedure linkage table.
                                                    /// The d_val member contains either DT_REL or DT_RELA.
                         DEBUG, 21,                 /// Reserved for debugger use.
                         TEXT_REL, 22,              /// The presence of this dynamic table entry signals that the
                                                    /// relocation table contains relocations for a non-writable
                                                    /// segment.
                         JUMP_REL, 23,              /// Address of the relocations associated with the procedure
                                                    /// linkage table.
                         BIND_NOW, 24,              /// The presence of this dynamic table entry signals that the
                                                    /// dynamic loader should process all relocations for this object
                                                    /// before transferring control to the program.
                         INITIALIZE_ARRAY, 25,      /// Pointer to an array of pointers to initialization functions.
                         TERMINATION_ARRAY, 26,     /// Pointer to an array of pointers to termination functions.
                         INITIALIZE_SIZE, 27,       /// Size, in bytes, of the array of initialization functions.
                         TERMINATION_SIZE, 28,      /// Size, in bytes, of the array of termination functions.
                         PRE_INITIALIZE_ARRAY, 32,
                         PRE_INITIALIZE_SIZE, 33,
                         GNU_HASH, 0x6ffffef5,
                         VER_SYM, 0x6ffffff0,
                         VER_NEED, 0x6ffffffe,
                         VER_NEEDNUM, 0x6fffffff
        );

        struct Entry {
            DynLinkingTag tag;
            USizeT val;
        };

        static constexpr usize ENTRY_SIZE = sizeof(Entry);
        static constexpr u32 TYPE = SectionHeader<USizeT>::DYNAMIC_LINKING_TABLE;

        using TableT = SectionIterable<USizeT, Entry>;

        TableT get_table(MappedFileVisitor &visitor) { return TableT{*this, visitor}; }
    };
}

namespace elf32 {
    using SectionHeader = elf::SectionHeader<elf::u32>;
    using StringTableHeader = elf::StringTableHeader<elf::u32>;
    using SymbolTableHeader = elf::SymbolTableHeader<elf::u32>;
    using DynSymbolTableHeader = elf::DynSymbolTableHeader<elf::u32>;
    using RelocationTableHeader = elf::RelocationTableHeader<elf::u32>;
    using RelocationTableAddendHeader = elf::RelocationTableAddendHeader<elf::u32>;
    using DynLinkingTableHeader = elf::DynLinkingTableHeader<elf::u32>;
}

namespace elf64 {
    using SectionHeader = elf::SectionHeader<elf::u64>;
    using StringTableHeader = elf::StringTableHeader<elf::u64>;
    using SymbolTableHeader = elf::SymbolTableHeader<elf::u64>;
    using DynSymbolTableHeader = elf::DynSymbolTableHeader<elf::u64>;
    using RelocationTableHeader = elf::RelocationTableHeader<elf::u64>;
    using RelocationTableAddendHeader = elf::RelocationTableAddendHeader<elf::u64>;
    using DynLinkingTableHeader = elf::DynLinkingTableHeader<elf::u64>;
}


#endif //ELF_SECTION_HEADER_HPP

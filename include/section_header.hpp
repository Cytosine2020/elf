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
                         RELOCATION_ENTRIES_ADDEND, 4,  /// “Rela” type relocation entries
                         HASH_TABLE, 5,                 /// a symbol hash table
                         DYNAMIC_LINKING_TABLE, 6,      /// dynamic linking tables
                         NOTE, 7,                       /// note information
                         NO_BITS, 8,                    /// uninitialized space; does not occupy any space in the file
                         RELOCATION_ENTRIES, 9,         /// “Rel” type relocation entries
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
            stream << "ELF32SectionHeader {\n";
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

    using ELF32SectionHeader = SectionHeader<u32>;
    using ELF64SectionHeader = SectionHeader<u64>;

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

            char *get_str(usize index) const {
                if (index >= header.size) return nullptr;
                char *str = reinterpret_cast<char *>(visitor.trusted_address(header.offset + index));
                if (strnlen(str, header.size - index) == (header.size - index)) return nullptr;
                return str;
            }
        };

        static constexpr u32 TYPE = SectionHeader<USizeT>::STRING_TABLE;
        static constexpr usize ENTRY_SIZE = 0;

        StringTable get_string_table(MappedFileVisitor &visitor) { return StringTable{*this, visitor}; }
    };

    using ELF32StringTableHeader = StringTableHeader<u32>;
    using ELF64StringTableHeader = StringTableHeader<u64>;

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
    class SymbolTableHeader : public SectionHeader<USizeT> {
    public:
        elf_enum_display(SymbolBinding, u8, 3,
                         LOCAL, 0,
                         GLOBAL, 1,
                         WEAK, 2
        );

        elf_enum_display(SymbolType, u8, 5,
                         NO_TYPE, 0,
                         OBJECT, 1,
                         FUNC, 2,
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
            ///     in the low-order four bits. The processor-independent binding attributes are listed in Table 14,
            ///     and the processor-independent values for symbol type are listed in Table 15. An STT_FILE symbol
            ///     must have STB_LOCAL binding, its section index must be SHN_ABS, and it must precede all other local
            ///     symbols for the file.
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
        };

        class SymbolTable {
        private:
            SymbolTableHeader &header;
            MappedFileVisitor &visitor;

        public:
            SymbolTable(SymbolTableHeader &header, MappedFileVisitor &visitor) :
                    header{header}, visitor{visitor} {}

            ArrayIterator<SymbolTableEntry> begin() const {
                SymbolTableEntry *ptr = reinterpret_cast<SymbolTableEntry *>(visitor.trusted_address(header.offset));
                return ArrayIterator<SymbolTableEntry>{ptr, header.entry_size};
            }

            ArrayIterator<SymbolTableEntry> end() const {
                SymbolTableEntry *ptr = reinterpret_cast<SymbolTableEntry *>(visitor.trusted_address(
                        header.offset + header.size));
                return ArrayIterator<SymbolTableEntry>{ptr, header.entry_size};
            }
        };

        static constexpr u32 TYPE = SectionHeader<USizeT>::SYMBOL_TABLE;
        static constexpr usize ENTRY_SIZE = sizeof(SymbolTableEntry);

        SymbolTable get_symbol_table(MappedFileVisitor &visitor) { return SymbolTable{*this, visitor}; }
    };

    using ELF32SymbolTableHeader = SymbolTableHeader<u32>;
    using ELF64SymbolTableHeader = SymbolTableHeader<u64>;

    template <typename USizeT>
    class RelocationTableHeader : public SectionHeader<USizeT> {
    public:
        
    };

    using ELF32RelocationTableHeader = RelocationTableHeader<u32>;
    using ELF64RelocationTableHeader = RelocationTableHeader<u64>;
}


#endif //ELF_SECTION_HEADER_HPP

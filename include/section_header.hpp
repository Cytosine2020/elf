#ifndef ELF_SECTION_HEADER_HPP
#define ELF_SECTION_HEADER_HPP


namespace elf {
    template<typename USizeT>
    class SectionHeader {
    public:
        template<typename T>
        static T *cast(SectionHeader *self, MappedFileVisitor &visitor) {
            if (self->section_type != T::TYPE) return nullptr;
            if (self->entry_size < T::ENTRY_SIZE) return nullptr;
            if (self->entry_size != 0 && self->size % self->entry_size != 0) return nullptr;
            if (!visitor.check_address(self->offset, self->size)) return nullptr;

            return reinterpret_cast<T *>(self);
        }

        enum SectionHeaderType : u32 {
            Null = 0,                       /// Marks an unused section header
            ProgramBits = 1,                /// Contains information defined by the program
            SymbolTable = 2,                /// Contains a linker symbol table
            StringTable = 3,                /// Contains a string table
            RelocationEntriesAddend = 4,    /// Contains “Rela” type relocation entries
            HashTable = 5,                  /// Contains a symbol hash table
            DynamicLinkingTable = 6,        /// Contains dynamic linking tables
            Note = 7,                       /// Contains note information
            NoBits = 8,                     /// Contains uninitialized space; does not occupy any space in the file
            RelocationEntries = 9,          /// Contains “Rel” type relocation entries
            SharedLibrary = 10,             /// Reserved
            DynamicSymbol = 11,             /// Contains a dynamic loader symbol table
            InitializeArray = 14,           /// an array of pointers to initialization functions
            TerminationArray = 15,          /// an array of pointers to termination functions
            PreInitializeArray = 16,        /// an array of pointers to pre-initialization functions
        };

        friend std::ostream &operator<<(std::ostream &stream, SectionHeaderType self) {
            switch (self) {
                case Null:
                    stream << "Null";
                    break;
                case ProgramBits:
                    stream << "ProgramBits";
                    break;
                case SymbolTable:
                    stream << "SymbolTable";
                    break;
                case StringTable:
                    stream << "StringTable";
                    break;
                case RelocationEntriesAddend:
                    stream << "RelocationEntriesAddend";
                    break;
                case HashTable:
                    stream << "HashTable";
                    break;
                case DynamicLinkingTable:
                    stream << "DynamicLinkingTable";
                    break;
                case Note:
                    stream << "Note";
                    break;
                case NoBits:
                    stream << "NoBits";
                    break;
                case RelocationEntries:
                    stream << "RelocationEntries";
                    break;
                case SharedLibrary:
                    stream << "SharedLibrary";
                    break;
                case DynamicSymbol:
                    stream << "DynamicSymbol";
                    break;
                case InitializeArray:
                    stream << "InitializeArray";
                    break;
                case TerminationArray:
                    stream << "TerminationArray";
                    break;
                case PreInitializeArray:
                    stream << "PreInitializeArray";
                    break;
                default:
                    stream << '[' << static_cast<size_t>(self) << ']';
            }

            return stream;
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

        friend std::ostream &operator<<(std::ostream &stream, const SectionHeader &self) {
            stream << "ELF32SectionHeader {\n";
            stream << "\tname: " << self.name << ",\n";
            stream << "\tsection_type: " << self.section_type << ",\n";
            stream << "\tflags: " << self.flags << ",\n";
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
    class StringTableSectionHeader : public SectionHeader<USizeT> {
    public:
        class StringTable {
        private:
            StringTableSectionHeader &header;
            MappedFileVisitor &visitor;

        public:
            StringTable(StringTableSectionHeader &header, MappedFileVisitor &visitor) :
                    header{header}, visitor{visitor} {}

            char *get_str(size_t index) const {
                if (index >= header.size) return nullptr;
                char *str = reinterpret_cast<char *>(visitor.trusted_address(header.offset + index));
                if (strnlen(str, header.size - index) == (header.size - index)) return nullptr;
                return str;
            }
        };

        static constexpr u32 TYPE = SectionHeader<USizeT>::StringTable;
        static constexpr usize ENTRY_SIZE = 0;

        StringTable get_string_table(MappedFileVisitor &visitor) { return StringTable{*this, visitor}; }
    };

    using ELF32StringTableSectionHeader = StringTableSectionHeader<u32>;
    using ELF64StringTableSectionHeader = StringTableSectionHeader<u64>;

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
    class SymbolTableSectionHeader : public SectionHeader<USizeT> {
    public:
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


        };

        class SymbolTable {
        private:
            SymbolTableSectionHeader &header;
            MappedFileVisitor &visitor;

        public:
            SymbolTable(SymbolTableSectionHeader &header, MappedFileVisitor &visitor) :
                    header{header}, visitor{visitor} {}

            ArrayIterator <SymbolTableEntry> begin() const {
                SymbolTableEntry *ptr = reinterpret_cast<SymbolTableEntry *>(visitor.trusted_address(header.offset));
                return ArrayIterator<SymbolTableEntry>{ptr, header.entry_size};
            }

            ArrayIterator <SymbolTableEntry> end() const {
                SymbolTableEntry *ptr = reinterpret_cast<SymbolTableEntry *>(visitor.trusted_address(
                        header.offset + header.size));
                return ArrayIterator<SymbolTableEntry>{ptr, header.entry_size};
            }
        };

        static constexpr u32 TYPE = SectionHeader<USizeT>::SymbolTable;
        static constexpr usize ENTRY_SIZE = sizeof(SymbolTableEntry);

        SymbolTable get_symbol_table(MappedFileVisitor &visitor) { return SymbolTable{*this, visitor}; }
    };

    using ELF32SymbolTableSectionHeader = SymbolTableSectionHeader<u32>;
    using ELF64SymbolTableSectionHeader = SymbolTableSectionHeader<u64>;
}


#endif //ELF_SECTION_HEADER_HPP

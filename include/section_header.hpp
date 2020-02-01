#ifndef ELF_SECTION_HEADER_HPP
#define ELF_SECTION_HEADER_HPP


namespace elf {
    class ELF32SectionHeader {
    public:
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
        u32 flags;
        /// contains the virtual address of the beginning of the section in memory. If the section is
        /// not allocated to the memory image of the program, this field should be zero.
        u32 address;
        /// contains the offset, in bytes, of the beginning of the section contents in the file.
        u32 offset;
        /// contains the size, in bytes, of the section. Except for ShtNoBits sections, this is the
        /// amount of space occupied in the file.
        u32 size;
        /// contains the section index of an associated section. This field is used for several
        /// purposes, depending on the type of section, as explained in Table 10.
        u32 link;
        /// contains extra information about the section. This field is used for several purposes,
        /// depending on the type of section, as explained in Table 11.
        u32 info;
        /// contains the required alignment of the section. This field must be a power of two.
        u32 alignment;
        /// contains the size, in bytes, of each entry, for sections that contain fixed-size entries.
        /// Otherwise, this field contains zero.
        u32 entry_size;

        friend std::ostream &operator<<(std::ostream &stream, const ELF32SectionHeader &self) {
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

    class ELF32StringTableSectionHeader : public ELF32SectionHeader {
    public:
        template<typename VisitorT>
        class ELF32StringTable {
        private:
            ELF32StringTableSectionHeader &header;
            VisitorT &visitor;

        public:
            ELF32StringTable(ELF32StringTableSectionHeader &header, VisitorT &visitor) :
                    header{header}, visitor{visitor} {}

            char *get_str(size_t index) const {
                return reinterpret_cast<char *>(visitor.address(header.offset + index));
            }
        };

        template<typename VisitorT>
        ELF32StringTable<VisitorT> get_string_table(VisitorT &visitor) {
            return ELF32StringTable{*this, visitor};
        }
    };

    template<>
    ELF32StringTableSectionHeader *
    dyn_cast<ELF32StringTableSectionHeader, ELF32SectionHeader>(ELF32SectionHeader *self) {
        return self->section_type == ELF32SectionHeader::StringTable ?
               reinterpret_cast<ELF32StringTableSectionHeader *>(self) : nullptr;
    }
}


#endif //ELF_SECTION_HEADER_HPP

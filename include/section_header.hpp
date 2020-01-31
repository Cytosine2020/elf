#ifndef ELF_SECTION_HEADER_HPP
#define ELF_SECTION_HEADER_HPP


namespace elf {
    class ELF32SectionHeader {
    public:
        enum SectionHeaderType : u32 {
            Null = 0,           /// Marks an unused section header
            ProgramBits = 1,    /// Contains information defined by the program
            SymbolTable = 2,    /// Contains a linker symbol table
            StringTable = 3,    /// Contains a string table
            Rela = 4,           /// Contains “Rela” type relocation entries
            Hash = 5,           /// Contains a symbol hash table
            Dynamic = 6,        /// Contains dynamic linking tables
            Note = 7,           /// Contains note information
            NoBits = 8,         /// Contains uninitialized space; does not occupy any space in the file
            Rel = 9,            /// Contains “Rel” type relocation entries
            Shlib = 10,         /// Reserved
            DynamicSymbol = 11, /// Contains a dynamic loader symbol table
        };

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
}


#endif //ELF_SECTION_HEADER_HPP

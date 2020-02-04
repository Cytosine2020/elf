#ifndef ELF_PROGRAM_HEADER_HPP
#define ELF_PROGRAM_HEADER_HPP


namespace elf {
    template <typename USizeT>
    class ProgramHeader;

    template <>
    class ProgramHeader<u32> {
    public:
        enum ProgramHeaderType {
            Null = 0,
            Loadable = 1,
            DynamicLinkingTables = 2,
            InterpreterPathName = 3,
            Note = 4,
            SharedLibrary = 5,
            ProgramHeaderTable = 6,
        };

        friend std::ostream &operator<<(std::ostream &stream, const ProgramHeaderType &self) {
            switch (self) {
                case Null:
                    stream << "Null";
                    break;
                case Loadable:
                    stream << "Loadable";
                    break;
                case DynamicLinkingTables:
                    stream << "DynamicLinkingTables";
                    break;
                case InterpreterPathName:
                    stream << "InterpreterPathName";
                    break;
                case Note:
                    stream << "Note";
                    break;
                case SharedLibrary:
                    stream << "SharedLibrary";
                    break;
                case ProgramHeaderTable:
                    stream << "ProgramHeaderTable";
                    break;
                default:
                    stream << '[' << static_cast<size_t>(self) << ']';
            }

            return stream;
        }

        /// This member tells what kind of segment this array element describes or how to
        /// interpret the array element's information. Type values and their meanings appear
        /// below.
        ProgramHeaderType type;
        /// This member gives the offset from the beginning of the file at which the first byte of
        /// the segment resides.
        u32 offset;
        /// This member gives the virtual address at which the first byte of the segment resides
        /// in memory.
        u32 virtual_address;
        /// On systems for which physical addressing is relevant, this member is reserved for the
        /// segment's physical address. This member requires operating system specific
        /// information, which is described in the appendix at the end of Book III.
        u32 physical_address;
        /// This member gives the number of bytes in the file image of the segment; it may be
        /// zero.
        u32 file_size;
        /// This member gives the number of bytes in the memory image of the segment; it may be
        /// zero.
        u32 mem_size;
        /// This member gives flags relevant to the segment. Defined flag values appear below.
        u32 flags;
        /// Loadable process segments must have congruent values for `virtual_address` and
        /// `physical_address`, modulo the page size.This member gives the value to which the
        /// segments are aligned in memory and in the file. Values 0 and 1 mean that no alignment
        /// is required. Otherwise, `alignment` should be a positive, integral power of 2, and
        /// `address` should equal `offset`, modulo `alignment`.
        u32 alignment;

        friend std::ostream &operator<<(std::ostream &stream, const ProgramHeader &self) {
            stream << "ELF32ProgramHeader {\n";
            stream << "\ttype: " << self.type << ",\n";
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

    using ELF32ProgramHeader = ProgramHeader<u32>;
}


#endif //ELF_PROGRAM_HEADER_HPP

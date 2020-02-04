#ifndef ELF_ELF_HEADER_HPP
#define ELF_ELF_HEADER_HPP


#include <iostream>

#include "utility.hpp"
#include "section_header.hpp"
#include "program_header.hpp"


namespace elf {
    class ELF32Header {
    public:
        class ProgramIterator {
        private:
            size_t size;
            void *inner;

        public:
            explicit ProgramIterator(size_t size, ELF32ProgramHeader *inner)
                    : size{size}, inner{inner} {}

            bool operator!=(const ProgramIterator &other) const { return inner != other.inner; }

            ProgramIterator operator++() {
                inner = static_cast<u8 *>(inner) + size;
                return *this;
            }

            const ELF32ProgramHeader &operator*() const {
                return *reinterpret_cast<ELF32ProgramHeader *>(inner);
            }
        };

        class ProgramIterable {
        private:
            ELF32Header &header;
            MappedFileVisitor &visitor;

        public:
            explicit ProgramIterable(ELF32Header &header, MappedFileVisitor &visitor) :
                    header{header}, visitor{visitor} {}

            ProgramIterator begin() const {
                void *ptr = visitor.trusted_address(header.program_header_offset);
                return ProgramIterator{header.program_header_size, reinterpret_cast<ELF32ProgramHeader *>(ptr)};
            }

            ProgramIterator end() const {
                void *ptr = visitor.trusted_address(header.program_header_offset +
                                                    header.program_header_num * header.program_header_size);
                return ProgramIterator{header.program_header_size, reinterpret_cast<ELF32ProgramHeader *>(ptr)};
            }
        };

        class SectionIterator {
        private:
            void *inner;
            const size_t size;

        public:
            explicit SectionIterator(ELF32SectionHeader *inner, size_t size) :
                    inner{inner}, size{size} {}

            bool operator!=(const SectionIterator &other) const { return inner != other.inner; }

            SectionIterator operator++() {
                inner = static_cast<u8 *>(inner) + size;
                return *this;
            }

            SectionIterator operator++(int) {
                SectionIterator ret = *this;
                inner = static_cast<u8 *>(inner) + size;
                return ret;
            }

            ELF32SectionHeader &operator*() const {
                return *reinterpret_cast<ELF32SectionHeader *>(inner);
            }

            ELF32SectionHeader *operator->() const {
                return reinterpret_cast<ELF32SectionHeader *>(inner);
            }

            ELF32SectionHeader &operator[](size_t index) const {
                u8 *ptr = static_cast<u8 *>(inner) + size * index;
                return *reinterpret_cast<ELF32SectionHeader *>(ptr);
            }
        };

        class SectionIterable {
        private:
            ELF32Header &header;
            MappedFileVisitor &visitor;

        public:
            explicit SectionIterable(ELF32Header &header, MappedFileVisitor &visitor) :
                    header{header}, visitor{visitor} {}

            SectionIterator begin() const {
                void *ptr = visitor.trusted_address(header.section_header_offset);
                return SectionIterator{reinterpret_cast<ELF32SectionHeader *>(ptr), header.section_header_size};
            }

            SectionIterator end() const {
                void *ptr = visitor.trusted_address(
                        header.section_header_offset + header.section_header_num * header.section_header_size);
                return SectionIterator{reinterpret_cast<ELF32SectionHeader *>(ptr), header.section_header_size};
            }

            ELF32SectionHeader &operator[](size_t index) const {
                if (index >= header.section_header_num) elf_abort("index out of boundary!");

                return begin()[index];
            }

            size_t length() const { return header.section_header_num; }
        };

        enum ELFClass : u8 {
            Bits32 = 1,
            Bits64 = 2,
        };

        friend std::ostream &operator<<(std::ostream &stream, ELFClass self) {
            switch (self) {
                case Bits32:
                    stream << "Bits32";
                    break;
                case Bits64:
                    stream << "Bits64";
                    break;
                default:
                    stream << '[' << static_cast<size_t>(self) << ']';
            }

            return stream;
        }

        enum DataEncoding : u8 {
            LittleEndian = 1,
            BigEndian = 2,
        };

        friend std::ostream &operator<<(std::ostream &stream, DataEncoding self) {
            switch (self) {
                case LittleEndian:
                    stream << "LittleEndian";
                    break;
                case BigEndian:
                    stream << "BigEndian";
                    break;
                default:
                    stream << '[' << static_cast<size_t>(self) << ']';
            }

            return stream;
        }

        enum OsAbi : u8 {
            SystemV = 0,
            HpUx = 1,
            Standalone = 255,
        };

        friend std::ostream &operator<<(std::ostream &stream, OsAbi self) {
            switch (self) {
                case SystemV:
                    stream << "SystemV";
                    break;
                case HpUx:
                    stream << "HpUx";
                    break;
                case Standalone:
                    stream << "Standalone";
                    break;
                default:
                    stream << '[' << static_cast<size_t>(self) << ']';
            }

            return stream;
        }

        enum ObjectFileType : u8 {
            ObjectNone = 0,
            Relocatable = 1,
            Executable = 2,
            Shared = 3,
            Core = 4,
        };

        friend std::ostream &operator<<(std::ostream &stream, ObjectFileType self) {
            switch (self) {
                case ObjectNone:
                    stream << "None";
                    break;
                case Relocatable:
                    stream << "Relocatable";
                    break;
                case Executable:
                    stream << "Executable";
                    break;
                case Shared:
                    stream << "Shared";
                    break;
                case Core:
                    stream << "Core";
                    break;
                default:
                    stream << '[' << static_cast<size_t>(self) << ']';
            }

            return stream;
        }

        enum MachineType : u16 {
            MachineNone = 0,        /// No machine
            SPARC = 2,              /// SPARC
            Intel_80386 = 3,        /// Intel Architecture
            Motorola_68000 = 4,     /// Motorola 68000
            Motorola_88000 = 5,     /// Motorola 88000
            Intel_80860 = 6,        /// Intel 80860
            MIPS_RS3000_BE = 8,     /// MIPS RS3000 Big-Endian
            MIPS_RS4000_BE = 10,    /// MIPS RS4000 Big-Endian
            RISCV = 243,            /// RISCV
        };

        friend std::ostream &operator<<(std::ostream &stream, MachineType self) {
            switch (self) {
                case MachineNone:
                    stream << "None";
                    break;
                case SPARC:
                    stream << "SPARC";
                    break;
                case Intel_80386:
                    stream << "Intel_80386";
                    break;
                case Motorola_68000:
                    stream << "Motorola_68000";
                    break;
                case Motorola_88000:
                    stream << "Motorola_88000";
                    break;
                case Intel_80860:
                    stream << "Intel_80860";
                    break;
                case MIPS_RS3000_BE:
                    stream << "MIPS_RS3000_BE";
                    break;
                case MIPS_RS4000_BE:
                    stream << "MIPS_RS4000_BE";
                    break;
                case RISCV:
                    stream << "RISCV";
                    break;
                default:
                    stream << '[' << static_cast<size_t>(self) << ']';
            }

            return stream;
        }

        static constexpr char MAGIC_0 = '\x7f';
        static constexpr char MAGIC_1 = 'E';
        static constexpr char MAGIC_2 = 'L';
        static constexpr char MAGIC_3 = 'F';

        static ELF32Header *read(MappedFileVisitor &visitor) {
            ELF32Header *header = reinterpret_cast<ELF32Header *>(visitor.address(0, sizeof(ELF32Header)));
            if (header == nullptr) return nullptr;

            // check magic number
            if (header->magic_number[0] != ELF32Header::MAGIC_0 ||
                header->magic_number[1] != ELF32Header::MAGIC_1 ||
                header->magic_number[2] != ELF32Header::MAGIC_2 ||
                header->magic_number[3] != ELF32Header::MAGIC_3)
                return nullptr;

            // check program header size and location in file
            if (header->program_header_size < sizeof(ELF32ProgramHeader)) return nullptr;
            if (!visitor.check_address(header->program_header_offset,
                                       header->program_header_num * header->program_header_size))
                return nullptr;

            // check section header size and location in file
            if (header->section_header_size < sizeof(ELF32SectionHeader)) return nullptr;
            if (!visitor.check_address(header->section_header_offset,
                                       header->section_header_num * header->section_header_size))
                return nullptr;

            // check string table index
            if (header->string_table_index > header->section_header_num) return nullptr;

            return header;
        }

        /// contain a “magic number,” identifying the file as an ELF object file. They contain the
        /// characters ‘\x7f’, ‘E’, ‘L’, and ‘F’, respectively.
        char magic_number[4];
        /// identifies the class of the object file, or its capacity.
        ///
        /// The class of the ELF file is independent of the data model assumed by the object code. The
        /// elf_class field identifies the file format; a processor-specific flag in the flags field,
        /// described below, may be used to identify the application’s data model if the processory
        /// supports multiple models.
        ELFClass elf_class;
        /// specifies the data encoding of the object file data structures.
        ///
        /// For the convenience of code that examines ELF object files at run time (e.g., the dynamic
        /// loader), it is intended that the data encoding of the object file will match that of the
        /// running program. For environments that support both byte orders, a processor-specific flag
        /// in the flags field may be used to identify the application’s operating mode.
        DataEncoding data_encoding;
        /// identifies the version of the object file format.
        u8 identification_version;
        /// identifies the operating system and ABI for which the object is prepared. Some fields in
        /// other ELF structures have flags and values that have environment-specific meanings; the
        /// interpretation of those fields is determined by the value of this field.
        OsAbi os_abi;
        /// identifies the version of the ABI for which the object is prepared. This field is used
        /// to distinguish among incompatible versions of an ABI. The interpretation of this version
        /// number is dependent on the ABI identified by the os_abi field.
        u8 abi_version;
        /// The remaining bytes are reserved for future use, and should be set to zero.
        u8 _reserve[7];
        /// identifies the object file type.
        ObjectFileType file_type;
        /// identifies the target architecture. These values are defined in the processor-specific
        /// supplements.
        MachineType machine_type;
        /// identifies the version of the object file format.
        u32 version;
        /// contains the virtual address of the program entry point. If there is no entry point, this
        /// field contains zero.
        u32 entry_point;
        /// contains the file offset, in bytes, of the program header table.
        u32 program_header_offset;
        /// contains the file offset, in bytes, of the section header table.
        u32 section_header_offset;
        /// contains processor-specific flags.
        u32 flags;
        /// contains the size, in bytes, of the ELF header.
        u16 elf_header_size;
        /// contains the size, in bytes, of a program header table entry.
        u16 program_header_size;
        /// contains the number of entries in the program header table.
        u16 program_header_num;
        /// contains the size, in bytes, of a section header table entry.
        u16 section_header_size;
        /// contains the number of entries in the section header table.
        u16 section_header_num;
        /// contains the section header table index of the section containing the section name string
        /// table. If there is no section name string table, this field has the value SHN_UNDEF.
        u16 string_table_index;

        ELF32Header(const ELF32Header &other) = delete;

        ELF32Header &operator=(const ELF32Header &other) = delete;

        friend std::ostream &operator<<(std::ostream &stream, const ELF32Header &self) {
            stream << "class ELF32Header {\n";
            stream << "\tmagic_number: " << self.magic_number << ",\n";
            stream << "\telf_class: " << self.elf_class << ",\n";
            stream << "\tdata_encoding: " << self.data_encoding << ",\n";
            stream << "\tidentification_version: " << static_cast<u32>(self.identification_version) << ",\n";
            stream << "\tos_abi: " << self.os_abi << ",\n";
            stream << "\tabi_version: " << static_cast<u32>(self.abi_version) << ",\n";
            stream << "\tfile_type: " << self.file_type << ",\n";
            stream << "\tmachine_type: " << self.machine_type << ",\n";
            stream << "\tversion: " << self.version << ",\n";
            stream << "\tentry_point: " << self.entry_point << ",\n";
            stream << "\tprogram_header_offset: " << self.program_header_offset << ",\n";
            stream << "\tsection_header_offset: " << self.section_header_offset << ",\n";
            stream << "\tflags: " << self.flags << ",\n";
            stream << "\telf_header_size: " << self.elf_header_size << ",\n";
            stream << "\tprogram_header_size: " << self.program_header_size << ",\n";
            stream << "\tprogram_header_num: " << self.program_header_num << ",\n";
            stream << "\tsection_header_size: " << self.section_header_size << ",\n";
            stream << "\tsection_header_num: " << self.section_header_num << ",\n";
            stream << "\tstring_table_index: " << self.string_table_index << ",\n";
            stream << '}';

            return stream;
        }

        ProgramIterable programs(MappedFileVisitor &visitor) { return ProgramIterable{*this, visitor}; }

        SectionIterable sections(MappedFileVisitor &visitor) { return SectionIterable{*this, visitor}; }
    };
}


#endif //ELF_ELF_HEADER_HPP

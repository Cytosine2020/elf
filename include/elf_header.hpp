#ifndef ELF_ELF_HEADER_HPP
#define ELF_ELF_HEADER_HPP


#include <iostream>

#include "elf_utility.hpp"
#include "section_header.hpp"
#include "program_header.hpp"


namespace elf {
    template<typename USizeT>
    class ELFHeader {
    public:
        using SectionHeaderT = SectionHeader<USizeT>;
        using ProgramHeaderT = ProgramHeader<USizeT>;

        class ProgramIterable {
        private:
            ELFHeader &header;
            MappedFileVisitor &visitor;

        public:
            using Iter = ArrayIterator<ProgramHeaderT>;

            explicit ProgramIterable(ELFHeader &header, MappedFileVisitor &visitor) :
                    header{header}, visitor{visitor} {}

            Iter begin() const {
                void *ptr = visitor.trusted_address(header.program_header_offset);
                return Iter{reinterpret_cast<ProgramHeaderT *>(ptr), header.program_header_size};
            }

            Iter end() const {
                void *ptr = visitor.trusted_address(header.program_header_offset +
                                                    header.program_header_num * header.program_header_size);
                return Iter{reinterpret_cast<ProgramHeaderT *>(ptr), header.program_header_size};
            }

            ProgramHeaderT &operator[](usize index) const {
                if (index >= header.program_header_num) elf_abort("index out of boundary!");
                return begin()[index];
            }
        };

        class SectionIterable {
        private:
            ELFHeader &header;
            MappedFileVisitor &visitor;

        public:
            using Iter = ArrayIterator<SectionHeaderT>;

            explicit SectionIterable(ELFHeader &header, MappedFileVisitor &visitor) :
                    header{header}, visitor{visitor} {}

            Iter begin() const {
                void *ptr = visitor.trusted_address(header.section_header_offset);
                return Iter{reinterpret_cast<SectionHeaderT *>(ptr), header.section_header_size};
            }

            Iter end() const {
                void *ptr = visitor.trusted_address(
                        header.section_header_offset + header.section_header_num * header.section_header_size);
                return Iter{reinterpret_cast<SectionHeaderT *>(ptr), header.section_header_size};
            }

            SectionHeaderT &operator[](usize index) const {
                if (index >= header.section_header_num) elf_abort("index out of boundary!");
                return begin()[index];
            }
        };

        elf_enum_display(ELFClass, u8, 2,
                         ELF32, 1,
                         ELF64, 2
        );

        elf_enum_display(DataEncoding, u8, 2,
                         DATA_LITTLE_ENDIAN, 1,
                         DATA_BIG_ENDIAN, 2
        );

        elf_enum_display(OsAbi, u8, 3,
                         SYSTEM_V, 0,
                         HP_UX, 1,
                         STAND_ALONE, 255
        );

        elf_enum_display(ObjectFileType, u8, 5,
                         OBJECT_NONE, 0,
                         RELOCATABLE, 1,
                         EXECUTABLE, 2,
                         SHARED, 3,
                         CORE, 4
        );

        elf_enum_display(MachineType, u16, 9,
                         MACHINE_NONE, 0,       /// No machine
                         SPARC, 2,              /// SPARC
                         INTEL_80386, 3,        /// Intel Architecture
                         MOTORORA_68000, 4,     /// Motorola 68000
                         MOTORORA_88000, 5,     /// Motorola 88000
                         INTEL_80860, 6,        /// Intel 80860
                         MIPS_RS3000_BE, 8,     /// MIPS RS3000 Big-Endian
                         MIPS_RS4000_BE, 10,    /// MIPS RS4000 Big-Endian
                         RISCV, 243             /// RISCV
        );

        static constexpr char MAGIC_0 = '\x7f';
        static constexpr char MAGIC_1 = 'E';
        static constexpr char MAGIC_2 = 'L';
        static constexpr char MAGIC_3 = 'F';

        static ELFHeader *read(MappedFileVisitor &visitor) {
            ELFHeader *header = reinterpret_cast<ELFHeader *>(visitor.address(0, sizeof(ELFHeader)));
            if (header == nullptr) return nullptr;

            // check magic number
            if (header->magic_number[0] != ELFHeader::MAGIC_0 ||
                header->magic_number[1] != ELFHeader::MAGIC_1 ||
                header->magic_number[2] != ELFHeader::MAGIC_2 ||
                header->magic_number[3] != ELFHeader::MAGIC_3)
                return nullptr;

            if (header->elf_header_size < sizeof(ELFHeader)) return nullptr;

            // check program header size and location in filex
            if (header->program_header_size < sizeof(ProgramHeaderT)) return nullptr;
            if (!visitor.check_address(header->program_header_offset,
                                       header->program_header_num * header->program_header_size))
                return nullptr;

            // check section header size and location in file
            if (header->section_header_size < sizeof(SectionHeaderT)) return nullptr;
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
        USizeT entry_point;
        /// contains the file offset, in bytes, of the program header table.
        USizeT program_header_offset;
        /// contains the file offset, in bytes, of the section header table.
        USizeT section_header_offset;
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

        ELFHeader(const ELFHeader &other) = delete;

        ELFHeader &operator=(const ELFHeader &other) = delete;

        friend std::ostream &operator<<(std::ostream &stream, const ELFHeader &self) {
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

        StringTableHeader<USizeT> *get_section_string_table_header(MappedFileVisitor &visitor) {
            return SectionHeader<USizeT>::template cast<StringTableHeader<USizeT>>(
                    &sections(visitor)[string_table_index], visitor);
        }

        /// if the section string table is invalid, this function will abort
        StringTableHeader<USizeT> *get_string_table_header(MappedFileVisitor &visitor) {
            auto *section_string_table_header = get_section_string_table_header(visitor);
            if (section_string_table_header == nullptr) neutron_abort("ELF file broken!");
            auto section_string_table = section_string_table_header->get_string_table(visitor);

            elf::ELF32StringTableHeader *string_table_header = nullptr;

            for (auto &section: sections(visitor)) {
                const char *name = section_string_table.get_str(section.name);

                if (strcmp(name, ".strtab") == 0) {
                    if (string_table_header != nullptr) return nullptr;
                    string_table_header = SectionHeader<USizeT>::template cast<StringTableHeader<USizeT>>(&section,
                                                                                                          visitor);
                    if (string_table_header == nullptr) return nullptr;
                }
            }

            return string_table_header;
        }
    };

    using ELF32Header = ELFHeader<u32>;
    using ELF64Header = ELFHeader<u64>;
}


#endif //ELF_ELF_HEADER_HPP

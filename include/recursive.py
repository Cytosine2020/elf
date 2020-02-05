#!python3


def recursive_macro_gen(name, arg, exp, num):
    print("#define ", name, "_1(", arg, ") ", exp, sep='')
    print()
    for i in range(2, num + 1):
        print("#define ", name, "_", i, "(", arg, ", ...) \\", sep='')
        print("    ", exp, " ", name, "_", i - 1, "(__VA_ARGS__)", sep='')
        print()


if __name__ == "__main__":
    RECURSIVE_NUM = 128

    recursive_macro_gen("_elf_enum_display_attribute_helper",
                        "a_0, v_0",
                        "a_0 = v_0,",
                        RECURSIVE_NUM)

    recursive_macro_gen("_elf_enum_display_display_helper",
                        "a_0, _",
                        "case a_0: stream << #a_0; break;",
                        RECURSIVE_NUM)

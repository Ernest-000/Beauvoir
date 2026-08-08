"""
This script extract all function from the include libraries (SYM_MODULE_PATH).
From thoses functions, it will generate all symbols importations in (SYM_PATH)
"""

# the function'sname where all the symbols will be imported
SYM_FUNCTION_SIG = ("bvr_load_default_sym")

# the macro signature 
SYM_MACRO_SIG = ("BVR_ADD_SYM({0}, {1})")
SYM_TCC_SIG = ("tcc_add_symbol({0}, \"{1}\", {2})")

# function include symbol (in the line)
SYM_STARTSWITH = ("extern")

# function exclusion symbol (in the name)
SYM_NOT_STARTSWITH = ("__")

# destination path
SYM_PATH = "include/bvr/script/sym.h"

# source path
SYM_MODULE_PATH = "scripts/include/"

# file list
SYM_MODULES = [
    "assert.h",
    "ctype.h",
    "float.h",
    "lbvr.h",
    "limits.h",
    "math.h",
    "stdarg.h",
    "stddef.h",
    "stdint.h",
    "stdio.h",
    "string.h",
    "time.h"
]

# user implemented functions
SYM_USER_FUNC = [
    ("stdin", "&stdin"),
    ("stderr", "&stderr"),
    ("stdout", "&stdout")
]

def file_read_n_lines(f, n, offset=0):
    text = ""
    f.seek(0)

    for _ in range(offset): f.readline()
    for _ in range(n): text += f.readline()
    
    return text

def file_insert_lines(f, lines, offset=0, end_offset=0):
    f.seek(0)
    line_c = sum(1 for _ in f) + 1

    chucka = file_read_n_lines(f, offset - 1)
    chuckb = file_read_n_lines(f, line_c - end_offset, end_offset)

    f.seek(0)
    f.truncate(0)
    f.write(chucka)
    f.write('\n')
    f.writelines(lines)
    f.write('\n')
    f.write(chuckb)

class cfunction:
    def __init__(self, declaration:str):
        self.return_type = ""
        self.name = ""
        self.params = []
        self.parse_function_name(declaration)

    def parse_function_name(self, declaration:str):
        declaration = declaration.strip()

        # detect false positive
        if not declaration.startswith(SYM_STARTSWITH):
            return ""

        f_as_array = declaration.split(" ")
        self.return_type = f_as_array[1]

        brace_index = 2
        for i in range(2, len(f_as_array) - 1):
            if "(" in f_as_array[i]: brace_index = i

        expression = " ".join(f_as_array[brace_index:])

        if(expression.startswith("*")):
            self.return_type += "*"
            expression = expression[1:]

        self.name = expression.split('(')[0]

    def __str__(self):
        return self.name

def is_line_function(line:str) -> bool:
    return line.strip().startswith(SYM_STARTSWITH) and '(' in line and ';'

def validate_func(f:cfunction):
    return not f.name.startswith(SYM_NOT_STARTSWITH)

def get_functions_from_module(mod:str):
    functions = []
    with open(SYM_MODULE_PATH + mod) as f:
        for line in f:
            if is_line_function(line):
                functions.append(cfunction(line))
    return functions

## start point
found_funcs = []


## find the insert point
insert_line = 0
insert_end_line = 0
with open(SYM_PATH, "r+") as f:
    line = "" 
    line_c = 0
    while True:
        line = f.readline()
        line_c += 1

        if line == "": break
        if SYM_FUNCTION_SIG in line:
            if('{' in line): insert_line = line_c + 1
            else: insert_line = line_c + 2
        if insert_line > 0 and '}' in line:
            insert_end_line = line_c - 1
            break

    # clear function's content
    """remove_from_file(f, insert_line, insert_end_line)
    num_lines = sum(1 for _ in f) + 1

    print(insert_line, insert_end_line)

    end = file_read_n_lines(f, num_lines - insert_end_line, insert_end_line)
    f.truncate(f.tell())
    f.seek(0)
    for _ in range(insert_line): f.readline()

    for func in found_funcs:
        func_str = SYM_MACRO_SIG.format("((TCCState*)_s)", func.name)
        f.write("\t{0};\n".format(func_str))

    f.write(end)"""

    flines = []
    for mod in SYM_MODULES:
        flines += "\n\t// {0}\n".format(mod)
        for func in get_functions_from_module(mod):
            if(validate_func(func)):
                func_str = SYM_MACRO_SIG.format("((TCCState*)_s)", func.name)
                flines += "\t{0};\n".format(func_str)

    flines += "\n\t// user\n"
    for func in SYM_USER_FUNC:
        func_str = SYM_TCC_SIG.format("((TCCState*)_s)", func[0], func[1])
        flines += "\t{0};\n".format(func_str)

    file_insert_lines(f, flines, insert_line, insert_end_line)

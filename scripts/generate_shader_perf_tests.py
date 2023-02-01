#!/usr/bin/python3

import os
import sys
from pathlib import Path


def write_program(program_file_name):
    result = ""
    with open(program_file_name, 'r') as program_file:
        result += "{\n"

        result += "\"{}\",\n".format(Path(program_file_name).stem)

        result += "{"
        for line in program_file.readlines():
            line_split = line.split(":")
            assert len(line_split) == 2
            result += "{"
            result += line_split[0]
            result += ",\n"

            shader_file_path = os.path.join(Path(program_file_name).parent, line_split[1].strip())
            with open(shader_file_path, 'r') as shader_file:
                result += "R\"({})\"".format(shader_file.read())
            result += "},"
        result += "},\n"

        result += "},\n"
    return result


def main():
    result = """struct ProgramInfo
        {
            const char *name;
            std::map<GLenum, const char *> shaders;
        };
        static const ProgramInfo kPrograms[] = {"""

    for program_file_name in sys.argv[1:]:
        result += write_program(program_file_name)

    result += "};\n"

    print(result)
    return 0


if __name__ == '__main__':
    sys.exit(main())

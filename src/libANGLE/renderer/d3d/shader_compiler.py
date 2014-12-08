import json
import os
import sys
import argparse

def get_input_file(data, input_dir):
    return os.path.join(input_dir, data["input_file"])

def compile(data, input_dir, output_file, compiler_path, debug):
    debug_flags = ""
    if (debug):
        debug_flags = "/Zi /Od"

    cmd = "\"%s\" /nologo %s /E %s /T %s /Fh %s %s" % \
        (compiler_path, debug_flags, data["entry_point"], data["type"], output_file, get_input_file(data, input_dir))
    print cmd

    return os.system(cmd)

def read_data(path):
    file = open(path, "r")
    return json.load(file)

def main(argv):
    parser = argparse.ArgumentParser(description="Compile HLSL shaders.")
    parser.add_argument("input_file")
    parser.add_argument("--debug", action='store_true')
    parser.add_argument("--compiler")
    parser.add_argument("output_file")
    arguments = parser.parse_args(argv)

    data = read_data(arguments.input_file)
    return compile(data, os.path.dirname(arguments.input_file), arguments.output_file, arguments.compiler, arguments.debug)

if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))

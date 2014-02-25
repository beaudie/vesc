import os
import re
import sys

def ReadFileAsLines(filename):
    """Reads a file, removing blank lines and lines that start with #"""
    file = open(filename, "r")
    raw_lines = file.readlines()
    file.close()
    lines = []
    for line in raw_lines:
        line = line.strip()
        if len(line) > 0 and not line.startswith("#"):
            lines.append(line)
    return lines

def CleanTestName(testName):
    replacements = { "dEQP-": "", ".*": "", ".":"_", }
    cleanName = testName
    for replaceKey in replacements:
        cleanName = cleanName.replace(replaceKey, replacements[replaceKey])
    return cleanName

def GenerateTests(outFile, testNames):
    ''' Remove duplicate tests '''
    testNames = list(set(testNames))

    outFile.write("#include \"deqp_test.h\"\n\n")

    for test in testNames:
        outFile.write("TEST(deqp, " + CleanTestName(test) + ")\n")
        outFile.write("{\n")
        outFile.write("    std::string output;\n")
        outFile.write("    bool result = RunDEQPTest(\"" + test + "\", output);\n")
        outFile.write("    EXPECT_TRUE(result) << output;\n")
        outFile.write("}\n\n")

def main(argv):
    tests = ReadFileAsLines(argv[0])
    output = open(argv[1], 'wb')
    GenerateTests(output, tests)
    output.close()

    return 0

if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))

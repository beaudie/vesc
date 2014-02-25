import zipfile
import sys
import os.path
import os

def unzip(source, dest, listOnly):
    zfile = zipfile.ZipFile(source, 'r')
    filelist = [ ]
    for name in zfile.namelist():
        (dirname, filename) = os.path.split(name)

        if listOnly:
            if filename != '':
                filelist.append(os.path.join(dest, name))
        else:
            fullOuputDir = os.path.join(dest, dirname)
            if not os.path.exists(fullOuputDir):
                os.mkdir(fullOuputDir)

            if filename != '':
                zfile.extract(name, fullOuputDir)

    zfile.close()
    if filelist:
        filelist.sort()
        for file in filelist:
            print file.replace("\\", "/")

def main(argv):
    if len(argv) < 2:
        return 0
    sourceZip = argv[0].replace("\"", "")
    extractDest = argv[1].replace("\"", "")
    listOnly = len(argv) >= 3 and argv[2] == '--listfiles'

    unzip(sourceZip, extractDest, listOnly)
    return 0

if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))

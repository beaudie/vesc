#!/bin/sh

CLANG_FORMAT=clang-format

FILES=$(find src | grep -v third_party | grep -E "\.(c|h|cpp|hpp)$")
PATCH="/tmp/clang-format-static-check"$(date +%s)

# reference: https://github.com/godotengine/godot/blob/master/misc/travis/clang-format.sh
for file in $FILES; do
    $CLANG_FORMAT -style=file $file | diff -u $file - >> $PATCH
done

if [ ! -s $PATCH ] ; then
    rm -f $PATCH
    exit 0
fi

printf "\n*** The following differences were found between the code to commit "
printf "and the clang-format rules:\n\n"
cat $PATCH
rm -f $PATCH
exit 1

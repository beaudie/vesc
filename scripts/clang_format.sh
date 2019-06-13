#!/bin/sh

# Copyright 2019 Google Inc.  All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

CLANG_FORMAT=clang-format

FILES=$(find src | grep -v third_party | grep -E "\.(c|h|cpp|hpp)$")

for file in $FILES; do
    echo "Apply clang-format on $file"
    $CLANG_FORMAT -i -style=file $file
done

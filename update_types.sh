#!/bin/bash
git ls-files | grep -E '\.(c|cpp|h|hpp|java|json|py)$' \
             | xargs -P 8 -d '\n' sed -b -i 's|\<deInt8\>|int8_t|g;
                                      s|\<deUint8\>|uint8_t|g;
                                      s|\<deInt16\>|int16_t|g;
                                      s|\<deUint16\>|uint16_t|g;
                                      s|\<deInt32\>|int32_t|g;
                                      s|\<deUint32\>|uint32_t|g;
                                      s|\<deInt64\>|int64_t|g;
                                      s|\<deUint64\>|uint64_t|g;
                                      s|\<deIntptr\>|intptr_t|g;
                                      s|\<deUintptr\>|uintptr_t|g;
                                      s|\<deBool\>|bool|g;
                                      s|\<DE_TRUE\>|true|g;
                                      s|\<DE_FALSE\>|false|g;
                                      s|::\<deGetFalse()|false|g;
                                      s|::\<deGetTrue()|true|g;
                                      s|\<deGetFalse()|false|g;
                                      s|\<deGetTrue()|true|g;
                                      s|\<DE_OFFSET_OF\>|offsetof|g'

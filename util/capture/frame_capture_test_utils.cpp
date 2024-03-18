//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// frame_capture_test_utils:
//   Helper functions for capture and replay of traces.
//

#include "frame_capture_test_utils.h"

#include "common/frame_capture_utils.h"
#include "common/string_utils.h"

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <fstream>

namespace angle
{

namespace
{
bool LoadJSONFromFile(const std::string &fileName, rapidjson::Document *doc)
{
    std::ifstream ifs(fileName);
    if (!ifs.is_open())
    {
        return false;
    }

    rapidjson::IStreamWrapper inWrapper(ifs);
    doc->ParseStream(inWrapper);
    return !doc->HasParseError();
}

// Branched from:
// https://crsrc.org/c/third_party/zlib/google/compression_utils_portable.cc;drc=9fc44ce454cc889b603900ccd14b7024ea2c284c;l=167
// Unmodified other than inlining ZlibStreamWrapperType and z_stream arg to access .msg
int GzipUncompressHelperPatched(Bytef *dest,
                                uLongf *dest_length,
                                const Bytef *source,
                                uLong source_length,
                                z_stream &stream)
{
    stream.next_in  = static_cast<z_const Bytef *>(const_cast<Bytef *>(source));
    stream.avail_in = static_cast<uInt>(source_length);
    if (static_cast<uLong>(stream.avail_in) != source_length)
        return Z_BUF_ERROR;

    stream.next_out  = dest;
    stream.avail_out = static_cast<uInt>(*dest_length);
    if (static_cast<uLong>(stream.avail_out) != *dest_length)
        return Z_BUF_ERROR;

    stream.zalloc = static_cast<alloc_func>(0);
    stream.zfree  = static_cast<free_func>(0);

    int err = inflateInit2(&stream, MAX_WBITS + 16);
    if (err != Z_OK)
        return err;

    err = inflate(&stream, Z_FINISH);
    if (err != Z_STREAM_END)
    {
        inflateEnd(&stream);
        if (err == Z_NEED_DICT || (err == Z_BUF_ERROR && stream.avail_in == 0))
            return Z_DATA_ERROR;
        return err;
    }
    *dest_length = stream.total_out;

    err = inflateEnd(&stream);
    return err;
}

static unsigned long crcbasic(const uint8_t *buf, size_t len)
{
    static unsigned long crc_table[] = {
        0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535,
        0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd,
        0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d,
        0x6ddde4eb, 0xf4d4b551, 0x83d385c7, 0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,
        0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4,
        0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
        0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59, 0x26d930ac,
        0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
        0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab,
        0xb6662d3d, 0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f,
        0x9fbfe4a5, 0xe8b8d433, 0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb,
        0x086d3d2d, 0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
        0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea,
        0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65, 0x4db26158, 0x3ab551ce,
        0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a,
        0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
        0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409,
        0xce61e49f, 0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
        0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739,
        0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
        0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1, 0xf00f9344, 0x8708a3d2, 0x1e01f268,
        0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0,
        0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8,
        0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
        0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef,
        0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703,
        0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7,
        0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d, 0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a,
        0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae,
        0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
        0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777, 0x88085ae6,
        0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
        0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d,
        0x3e6e77db, 0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5,
        0x47b2cf7f, 0x30b5ffe9, 0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605,
        0xcdd70693, 0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
        0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d};
    unsigned long crc = 0;
    crc               = (~crc) & 0xffffffff;
    while (len)
    {
        len--;
        crc = (crc >> 8) ^ crc_table[(crc ^ *buf++) & 0xff];
    }
    return crc ^ 0xffffffff;
}

#ifdef _MSC_VER
#    define zalign(x) __declspec(align(x))
#else
#    define zalign(x) __attribute__((aligned((x))))
#endif

static uint32_t _test_crc32_sse42_simd_(/* SSE4.2+PCLMUL */
                                        const unsigned char *buf,
                                        size_t len,
                                        uint32_t crc)
{
    /*
     * Definitions of the bit-reflected domain constants k1,k2,k3, etc and
     * the CRC32+Barrett polynomials given at the end of the paper.
     */
    static const uint64_t zalign(16) k1k2[] = {0x0154442bd4, 0x01c6e41596};
    static const uint64_t zalign(16) k3k4[] = {0x01751997d0, 0x00ccaa009e};
    static const uint64_t zalign(16) k5k0[] = {0x0163cd6124, 0x0000000000};
    static const uint64_t zalign(16) poly[] = {0x01db710641, 0x01f7011641};

    __m128i x0, x1, x2, x3, x4, x5, x6, x7, x8, y5, y6, y7, y8;

    /*
     * There's at least one block of 64.
     */
    x1 = _mm_loadu_si128((__m128i *)(buf + 0x00));
    x2 = _mm_loadu_si128((__m128i *)(buf + 0x10));
    x3 = _mm_loadu_si128((__m128i *)(buf + 0x20));
    x4 = _mm_loadu_si128((__m128i *)(buf + 0x30));

    x1 = _mm_xor_si128(x1, _mm_cvtsi32_si128(crc));

    x0 = _mm_load_si128((__m128i *)k1k2);

    buf += 64;
    len -= 64;

    /*
     * Parallel fold blocks of 64, if any.
     */
    while (len >= 64)
    {
        x5 = _mm_clmulepi64_si128(x1, x0, 0x00);
        x6 = _mm_clmulepi64_si128(x2, x0, 0x00);
        x7 = _mm_clmulepi64_si128(x3, x0, 0x00);
        x8 = _mm_clmulepi64_si128(x4, x0, 0x00);

        x1 = _mm_clmulepi64_si128(x1, x0, 0x11);
        x2 = _mm_clmulepi64_si128(x2, x0, 0x11);
        x3 = _mm_clmulepi64_si128(x3, x0, 0x11);
        x4 = _mm_clmulepi64_si128(x4, x0, 0x11);

        y5 = _mm_loadu_si128((__m128i *)(buf + 0x00));
        y6 = _mm_loadu_si128((__m128i *)(buf + 0x10));
        y7 = _mm_loadu_si128((__m128i *)(buf + 0x20));
        y8 = _mm_loadu_si128((__m128i *)(buf + 0x30));

        x1 = _mm_xor_si128(x1, x5);
        x2 = _mm_xor_si128(x2, x6);
        x3 = _mm_xor_si128(x3, x7);
        x4 = _mm_xor_si128(x4, x8);

        x1 = _mm_xor_si128(x1, y5);
        x2 = _mm_xor_si128(x2, y6);
        x3 = _mm_xor_si128(x3, y7);
        x4 = _mm_xor_si128(x4, y8);

        buf += 64;
        len -= 64;
    }

    /*
     * Fold into 128-bits.
     */
    x0 = _mm_load_si128((__m128i *)k3k4);

    x5 = _mm_clmulepi64_si128(x1, x0, 0x00);
    x1 = _mm_clmulepi64_si128(x1, x0, 0x11);
    x1 = _mm_xor_si128(x1, x2);
    x1 = _mm_xor_si128(x1, x5);

    x5 = _mm_clmulepi64_si128(x1, x0, 0x00);
    x1 = _mm_clmulepi64_si128(x1, x0, 0x11);
    x1 = _mm_xor_si128(x1, x3);
    x1 = _mm_xor_si128(x1, x5);

    x5 = _mm_clmulepi64_si128(x1, x0, 0x00);
    x1 = _mm_clmulepi64_si128(x1, x0, 0x11);
    x1 = _mm_xor_si128(x1, x4);
    x1 = _mm_xor_si128(x1, x5);

    /*
     * Single fold blocks of 16, if any.
     */
    while (len >= 16)
    {
        x2 = _mm_loadu_si128((__m128i *)buf);

        x5 = _mm_clmulepi64_si128(x1, x0, 0x00);
        x1 = _mm_clmulepi64_si128(x1, x0, 0x11);
        x1 = _mm_xor_si128(x1, x2);
        x1 = _mm_xor_si128(x1, x5);

        buf += 16;
        len -= 16;
    }

    /*
     * Fold 128-bits to 64-bits.
     */
    x2 = _mm_clmulepi64_si128(x1, x0, 0x10);
    x3 = _mm_setr_epi32(~0, 0, ~0, 0);
    x1 = _mm_srli_si128(x1, 8);
    x1 = _mm_xor_si128(x1, x2);

    x0 = _mm_loadl_epi64((__m128i *)k5k0);

    x2 = _mm_srli_si128(x1, 4);
    x1 = _mm_and_si128(x1, x3);
    x1 = _mm_clmulepi64_si128(x1, x0, 0x00);
    x1 = _mm_xor_si128(x1, x2);

    /*
     * Barret reduce to 32-bits.
     */
    x0 = _mm_load_si128((__m128i *)poly);

    x2 = _mm_and_si128(x1, x3);
    x2 = _mm_clmulepi64_si128(x2, x0, 0x10);
    x2 = _mm_and_si128(x2, x3);
    x2 = _mm_clmulepi64_si128(x2, x0, 0x00);
    x1 = _mm_xor_si128(x1, x2);

    /*
     * Return the crc32.
     */
    return _mm_extract_epi32(x1, 1);
}

bool UncompressData(const std::vector<uint8_t> &compressedData,
                    std::vector<uint8_t> *uncompressedData)
{
    uint32_t uncompressedSize =
        zlib_internal::GetGzipUncompressedSize(compressedData.data(), compressedData.size());

    uncompressedData->resize(uncompressedSize + 1);  // +1 to make sure .data() is valid
    uLong destLen = uncompressedSize;
    z_stream stream;
    int zResult =
        GzipUncompressHelperPatched(uncompressedData->data(), &destLen, compressedData.data(),
                                    static_cast<uLong>(compressedData.size()), stream);

    // static const char *crct =
    //     "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123";

    unsigned int simdSize = 16 * (uncompressedSize / 16);
    {
        unsigned long a = crcbasic(uncompressedData->data(), simdSize);
        for (int i = 0; i < 100; i++)
        {
            unsigned long b = crc32(0, uncompressedData->data(), simdSize);
            if (a != b)
            {
                printf("mismatch %d %lX %lX\n", i, a, b);
                break;
            }
        }

        for (int i = 0; i < 100; i++)
        {
            unsigned long b = ~_test_crc32_sse42_simd_(uncompressedData->data(), simdSize, ~0);
            if (a != b)
            {
                printf("mismatchT %d %lX %lX\n", i, a, b);
                break;
            }
        }
    }

    {
        std::vector<uint8_t> tempData;
        tempData.resize(simdSize);
        for (int i = 0; i < (int)simdSize; i++)
        {
            tempData[i] = i % 256;
        }
        unsigned long a2 = crcbasic(tempData.data(), simdSize);
        for (int i = 0; i < 100; i++)
        {
            unsigned long b2 = crc32(0, tempData.data(), simdSize);
            if (a2 != b2)
            {
                printf("mismatch2 %d %lX %lX\n", i, a2, b2);
                break;
            }
        }
    }
    // for (int j = 16; j < (int)uncompressedSize; j *= 2)
    // {
    //     for (int i = 0; i < 4; i++)
    //     {
    //         printf("crc check %d %d %lX %lX %lX %lX\n", j, i, crc32(0, uncompressedData->data(),
    //         j),
    //                crc32(0, (uint8_t *)crct, 4), crc32(0, (uint8_t *)crct, 84),
    //                crcbasic((uint8_t *)crct, 84));
    //     }
    // }

    if (zResult != Z_OK)
    {
        std::cerr << "Failure to decompressed binary data: " << zResult
                  << " msg=" << (stream.msg ? stream.msg : "nil") << "\n";
        fprintf(stderr,
                "next_in %p (input %p) avail_in %d total_in %lu next_out %p (output %p) avail_out "
                "%d total_out %ld adler %lX crc %lX crc_simd %lX\n",
                stream.next_in, compressedData.data(), stream.avail_in, stream.total_in,
                stream.next_out, uncompressedData->data(), stream.avail_out, stream.total_out,
                stream.adler, crc32(0, uncompressedData->data(), uncompressedSize),
                crc32(0, uncompressedData->data(), 16 * (uncompressedSize / 16)));

        return false;
    }

    return true;
}

void SaveDebugFile(const std::string &outputDir,
                   const char *baseFileName,
                   const char *suffix,
                   const std::vector<uint8_t> data)
{
    if (outputDir.empty())
    {
        return;
    }

    std::ostringstream path;
    path << outputDir << "/" << baseFileName << suffix;
    FILE *fp = fopen(path.str().c_str(), "wb");
    fwrite(data.data(), 1, data.size(), fp);
    fclose(fp);
}
}  // namespace

bool LoadTraceNamesFromJSON(const std::string jsonFilePath, std::vector<std::string> *namesOut)
{
    rapidjson::Document doc;
    if (!LoadJSONFromFile(jsonFilePath, &doc))
    {
        return false;
    }

    if (!doc.IsObject() || !doc.HasMember("traces") || !doc["traces"].IsArray())
    {
        return false;
    }

    // Read trace json into a list of trace names.
    std::vector<std::string> traces;

    rapidjson::Document::Array traceArray = doc["traces"].GetArray();
    for (rapidjson::SizeType arrayIndex = 0; arrayIndex < traceArray.Size(); ++arrayIndex)
    {
        const rapidjson::Document::ValueType &arrayElement = traceArray[arrayIndex];

        if (!arrayElement.IsString())
        {
            return false;
        }

        std::vector<std::string> traceAndVersion;
        angle::SplitStringAlongWhitespace(arrayElement.GetString(), &traceAndVersion);
        traces.push_back(traceAndVersion[0]);
    }

    *namesOut = std::move(traces);
    return true;
}

bool LoadTraceInfoFromJSON(const std::string &traceName,
                           const std::string &traceJsonPath,
                           TraceInfo *traceInfoOut)
{
    rapidjson::Document doc;
    if (!LoadJSONFromFile(traceJsonPath, &doc))
    {
        return false;
    }

    if (!doc.IsObject() || !doc.HasMember("TraceMetadata"))
    {
        return false;
    }

    const rapidjson::Document::Object &meta = doc["TraceMetadata"].GetObj();

    strncpy(traceInfoOut->name, traceName.c_str(), kTraceInfoMaxNameLen);
    traceInfoOut->contextClientMajorVersion = meta["ContextClientMajorVersion"].GetInt();
    traceInfoOut->contextClientMinorVersion = meta["ContextClientMinorVersion"].GetInt();
    traceInfoOut->frameEnd                  = meta["FrameEnd"].GetInt();
    traceInfoOut->frameStart                = meta["FrameStart"].GetInt();
    traceInfoOut->drawSurfaceHeight         = meta["DrawSurfaceHeight"].GetInt();
    traceInfoOut->drawSurfaceWidth          = meta["DrawSurfaceWidth"].GetInt();

    angle::HexStringToUInt(meta["DrawSurfaceColorSpace"].GetString(),
                           &traceInfoOut->drawSurfaceColorSpace);
    angle::HexStringToUInt(meta["DisplayPlatformType"].GetString(),
                           &traceInfoOut->displayPlatformType);
    angle::HexStringToUInt(meta["DisplayDeviceType"].GetString(), &traceInfoOut->displayDeviceType);

    traceInfoOut->configRedBits     = meta["ConfigRedBits"].GetInt();
    traceInfoOut->configGreenBits   = meta["ConfigGreenBits"].GetInt();
    traceInfoOut->configBlueBits    = meta["ConfigBlueBits"].GetInt();
    traceInfoOut->configAlphaBits   = meta["ConfigAlphaBits"].GetInt();
    traceInfoOut->configDepthBits   = meta["ConfigDepthBits"].GetInt();
    traceInfoOut->configStencilBits = meta["ConfigStencilBits"].GetInt();

    traceInfoOut->isBinaryDataCompressed = meta["IsBinaryDataCompressed"].GetBool();
    traceInfoOut->areClientArraysEnabled = meta["AreClientArraysEnabled"].GetBool();
    traceInfoOut->isBindGeneratesResourcesEnabled =
        meta["IsBindGeneratesResourcesEnabled"].GetBool();
    traceInfoOut->isWebGLCompatibilityEnabled = meta["IsWebGLCompatibilityEnabled"].GetBool();
    traceInfoOut->isRobustResourceInitEnabled = meta["IsRobustResourceInitEnabled"].GetBool();
    traceInfoOut->windowSurfaceContextId      = doc["WindowSurfaceContextID"].GetInt();

    if (doc.HasMember("RequiredExtensions"))
    {
        const rapidjson::Value &requiredExtensions = doc["RequiredExtensions"];
        if (!requiredExtensions.IsArray())
        {
            return false;
        }
        for (rapidjson::SizeType i = 0; i < requiredExtensions.Size(); i++)
        {
            std::string ext = std::string(requiredExtensions[i].GetString());
            traceInfoOut->requiredExtensions.push_back(ext);
        }
    }

    if (meta.HasMember("KeyFrames"))
    {
        const rapidjson::Value &keyFrames = meta["KeyFrames"];
        if (!keyFrames.IsArray())
        {
            return false;
        }
        for (rapidjson::SizeType i = 0; i < keyFrames.Size(); i++)
        {
            int frame = keyFrames[i].GetInt();
            traceInfoOut->keyFrames.push_back(frame);
        }
    }

    const rapidjson::Document::Array &traceFiles = doc["TraceFiles"].GetArray();
    for (const rapidjson::Value &value : traceFiles)
    {
        traceInfoOut->traceFiles.push_back(value.GetString());
    }

    traceInfoOut->initialized = true;
    return true;
}

TraceLibrary::TraceLibrary(const std::string &traceName, const TraceInfo &traceInfo)
{
    std::stringstream libNameStr;
    SearchType searchType = SearchType::ModuleDir;

#if defined(ANGLE_TRACE_EXTERNAL_BINARIES)
    // This means we are using the binary build of traces on Android, which are
    // not bundled in the APK, but located in the app's home directory.
    searchType = SearchType::SystemDir;
    libNameStr << "/data/user/0/com.android.angle.test/angle_traces/";
#endif  // defined(ANGLE_TRACE_EXTERNAL_BINARIES)
#if !defined(ANGLE_PLATFORM_WINDOWS)
    libNameStr << "lib";
#endif  // !defined(ANGLE_PLATFORM_WINDOWS)
    libNameStr << traceName;
    std::string libName = libNameStr.str();
    std::string loadError;
    mTraceLibrary.reset(OpenSharedLibraryAndGetError(libName.c_str(), searchType, &loadError));
    if (mTraceLibrary->getNative() == nullptr)
    {
        FATAL() << "Failed to load trace library (" << libName << "): " << loadError;
    }

    callFunc<SetupEntryPoints>("SetupEntryPoints", static_cast<angle::TraceCallbacks *>(this),
                               &mTraceFunctions);
    mTraceFunctions->SetTraceInfo(traceInfo);
    mTraceInfo = traceInfo;
}

uint8_t *TraceLibrary::LoadBinaryData(const char *fileName)
{
    std::ostringstream pathBuffer;
    pathBuffer << mBinaryDataDir << "/" << fileName;
    FILE *fp = fopen(pathBuffer.str().c_str(), "rb");
    if (fp == 0)
    {
        fprintf(stderr, "Error loading binary data file: %s\n", fileName);
        exit(1);
    }
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (mTraceInfo.isBinaryDataCompressed)
    {
        if (!strstr(fileName, ".gz"))
        {
            fprintf(stderr, "Filename does not end in .gz");
            exit(1);
        }

        std::vector<uint8_t> compressedData(size);
        size_t bytesRead = fread(compressedData.data(), 1, size, fp);
        if (bytesRead != static_cast<size_t>(size))
        {
            std::cerr << "Failed to read binary data: " << bytesRead << " != " << size << "\n";
            exit(1);
        }

        if (!UncompressData(compressedData, &mBinaryData))
        {
            // Workaround for sporadic failures https://issuetracker.google.com/296921272
            SaveDebugFile(mDebugOutputDir, fileName, ".gzdbg_input.gz", compressedData);
            SaveDebugFile(mDebugOutputDir, fileName, ".gzdbg_attempt1", mBinaryData);
            std::vector<uint8_t> uncompressedData;
            bool secondResult = UncompressData(compressedData, &uncompressedData);
            SaveDebugFile(mDebugOutputDir, fileName, ".gzdbg_attempt2", uncompressedData);
            if (!secondResult)
            {
                std::cerr << "Uncompress retry failed\n";
                exit(1);
            }
            std::cerr << "Uncompress retry succeeded, moving to mBinaryData\n";
            mBinaryData = std::move(uncompressedData);
        }
    }
    else
    {
        if (!strstr(fileName, ".angledata"))
        {
            fprintf(stderr, "Filename does not end in .angledata");
            exit(1);
        }
        mBinaryData.resize(size + 1);
        (void)fread(mBinaryData.data(), 1, size, fp);
    }
    fclose(fp);

    return mBinaryData.data();
}

}  // namespace angle

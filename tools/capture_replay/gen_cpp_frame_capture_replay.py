#!/usr/bin/python
#
# Copyright 2019 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# gen_cpp_frame_capture_replay.py:
#   generate frame capture replay cpp file and overwrite simple_texture_2d sample to do the replay

import sys
import os

template_capture_replay = """
# include "SampleApplication.h"

# include <cstring>

# include <iostream>
# include <unordered_map>

# include "util/shader_utils.h"

{includes}

class CaptureReplay : public SampleApplication
{{
  public:
    CaptureReplay(int argc, char **argv) : SampleApplication("CatureReplay", argc, argv) {{}}

    bool initialize() override {{
        return true;
    }}

    void destroy() override {{}}

    void draw() override {{
        {draw_calls}
        std::cout << "Replay Complete" << std::endl;
        exit();
    }}

  private:
    std::vector<uint8_t> gBinaryData;
    std::vector<uint8_t> gReadBuffer;
    std::vector<uint8_t> gClientArrays[16];
    std::unordered_map<GLuint, GLuint> gRenderbufferMap;


    void UpdateClientArrayPointer(int arrayIndex, const void *data, GLuint64 size)
    {{
        memcpy(gClientArrays[arrayIndex].data(), data, size);
    }}

    void UpdateRenderbufferID(GLuint id, GLsizei readBufferOffset)
    {{
        GLuint returnedID;
        memcpy(&returnedID, &gReadBuffer[readBufferOffset], sizeof(GLuint));
        gRenderbufferMap[id] = returnedID;
    }}

    void LoadBinaryData(const std::string &fname, size_t binaryDataSize)
    {{
        gBinaryData.resize(static_cast<int>(binaryDataSize));
        FILE *fp = fopen(fname.c_str(), "rb");
        fread(gBinaryData.data(), 1, static_cast<int>(binaryDataSize), fp);
        fclose(fp);
    }}
}};

int main(int argc, char **argv)
{{
    CaptureReplay app(argc, argv);
    return app.run();
}}
"""


def main(capture_dir, first_frame, last_frame):
    script_dir = os.path.dirname(os.path.realpath(__file__))
    src_root_dir = os.path.normpath(os.path.join(script_dir, "..", "..", "src"))

    capture_dir_abspath = os.path.normpath(os.path.join(os.getcwd(), capture_dir))

    capture_dir_include_path = os.path.relpath(capture_dir_abspath, src_root_dir)

    content = template_capture_replay.format(
        includes='\n'.join([
            '#include "%s/angle_capture_frame%s.cpp"' % (capture_dir_include_path,
                                                         str(frame_idx).zfill(3))
            for frame_idx in range(first_frame, last_frame + 1)
        ]),
        draw_calls='\n'.join([
            'ReplayFrame%d;\nswap();\n' % frame_idx
            for frame_idx in range(first_frame, last_frame + 1)
        ]),
    )

    output_file_path = os.path.join(src_root_dir, "..", "samples", "simple_texture_2d",
                                    "SimpleTexture2D.cpp")
    with open(output_file_path, 'w') as f:
        f.write(content)


if __name__ == '__main__':
    arguments = sys.argv[1:]
    capture_dir = arguments[0] if len(arguments) > 0 else '.'
    frame_start = arguments[1] if len(arguments) > 1 else 0
    frame_end = arguments[2] if len(arguments) > 2 else 99
    main(capture_dir, frame_start, frame_end)

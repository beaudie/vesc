#!/usr/bin/python
#
# Copyright 2019 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# gen_cpp_frame_capture_replay.py:
#   gen frame capture replay

import os

current_dir = os.path.dirname(os.path.realpath(__file__))

template_capture_replay = """
# include "SampleApplication.h"

# include <cstring>

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
    }}

  private:
    std::vector<uint8_t> gBinaryData;
    std::vector<uint8_t> gClientArrays[gl::MAX_VERTEX_ATTRIBS];
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


def main(first_frame, last_frame):
    content = template_capture_replay.format(
        includes='\n'.join([
            '#include "angle_capture_frame%s.cpp"' % str(frame_idx).zfill(3)
            for frame_idx in range(first_frame, last_frame + 1)
        ]),
        draw_calls='\n'.join(
            ['ReplayFrame%d' % frame_idx for frame_idx in range(first_frame, last_frame + 1)]),
    )

    with open(os.path.join(current_dir, "CaptureReplayAutoGen.cpp"), 'w') as f:
        f.write(content)


if __name__ == '__main__':
    main(0, 99)

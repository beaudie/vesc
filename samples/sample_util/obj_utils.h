//
// Copyright (c) 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Based on:
// https://github.com/LFY/lel-engine/blob/master/lel/OBJData.cpp

#ifndef SAMPLE_UTIL_OBJPARSE_H
#define SAMPLE_UTIL_OBJPARSE_H

#include <array>
#include <string>
#include <map>
#include <vector>

class ObjModel {
public:
    ObjModel() {}

    // Loads the specific .obj file,
    // extracts vertex attributes and
    // index buffer.
    ObjModel(const std::string &objFileName);

    // Our vertex attribute format
    struct VertexAttributes {
        float pos[3];
        float norm[3];
        float texcoord[2];
    };

    // Products: buffer of vertex attributes,
    // buffer of unsigned int indices
    std::vector<VertexAttributes> vertexData;
    std::vector<unsigned int> indexData;

private:

    struct VertexKey {
        unsigned int pos;
        unsigned int norm;
        unsigned int texcoord;
    };

    struct VertexKeyCompare {
        bool operator()(const VertexKey &a,
                        const VertexKey &b) const {
            if (a.pos != b.pos)
                return a.pos < b.pos;
            if (a.norm != b.norm)
                return a.norm < b.norm;
            if (a.texcoord != b.texcoord)
                return a.texcoord < b.texcoord;
            return false;
        }
    };

    std::vector<std::array<float, 3> > mPositionList;
    std::vector<std::array<float, 3> > mNormalList;
    std::vector<std::array<float, 2> > mTexcoordList;
    std::vector<std::array<uint32_t, 9> > mIndices;

    std::map<VertexKey, VertexAttributes, VertexKeyCompare> vertexDataMap;
    std::map<VertexKey, unsigned int, VertexKeyCompare> indexDataMap;
};


#endif // SAMPLE_UTILS_OBJPARSE_H


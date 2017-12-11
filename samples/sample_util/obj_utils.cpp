//
// Copyright (c) 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Based on:
// https://github.com/LFY/lel-engine/blob/master/lel/OBJData.cpp

#include "obj_utils.h"
#include "util.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

ObjModel::ObjModel(const std::string &objFileName) {

    std::ifstream stream(objFileName);

    if (!stream)
    {
        std::cerr << "error opening obj file " << objFileName << " for reading.\n";
        return;
    }

    float x, y, z;
    unsigned int p0, t0, n0, p1, t1, n1, p2, t2, n2;

    std::string line;

    while (std::getline(stream, line)) {
        if (sscanf(&line[0], "v %f %f %f", &x, &y, &z) == 3) {
            mPositionList.push_back({x, y, z});
        } else if (sscanf(&line[0], "vn %f %f %f", &x, &y, &z) == 3) {
            mNormalList.push_back({x, y, z});
        } else if (sscanf(&line[0], "vt %f %f", &x, &y) == 2) {
            mTexcoordList.push_back({x, 1.0f - y}); // OpenGL has flipped texcoords vs Blender
        } else if (sscanf(&line[0], "f %u/%u/%u %u/%u/%u %u/%u/%u",
                          &p0, &t0, &n0, &p1, &t1, &n1, &p2, &t2, &n2) == 9) {
            mIndices.push_back({p0, t0, n0,
                             p1, t1, n1,
                             p2, t2, n2});
        }
    }

    for (unsigned int i = 0; i < (unsigned int) mIndices.size(); i++) {
        unsigned int vertA = mIndices[i][0] - 1;
        unsigned int vertB = mIndices[i][3] - 1;
        unsigned int vertC = mIndices[i][6] - 1;

        unsigned int normA = mIndices[i][2] - 1;
        unsigned int normB = mIndices[i][5] - 1;
        unsigned int normC = mIndices[i][8] - 1;

        unsigned int texA = mIndices[i][1] - 1;
        unsigned int texB = mIndices[i][4] - 1;
        unsigned int texC = mIndices[i][7] - 1;

        VertexKey keyA = {vertA, normA, texA};
        VertexKey keyB = {vertB, normB, texB};
        VertexKey keyC = {vertC, normC, texC};

        VertexAttributes &currDataA = vertexDataMap[keyA];
        VertexAttributes &currDataB = vertexDataMap[keyB];
        VertexAttributes &currDataC = vertexDataMap[keyC];

        indexDataMap[keyA] = 0;
        indexDataMap[keyB] = 0;
        indexDataMap[keyC] = 0;

        unsigned int posAttribBytes = sizeof(float) * 3;
        unsigned int normAttribBytes = sizeof(float) * 3;
        unsigned int texcoordAttribBytes = sizeof(float) * 2;

        memcpy(&currDataA.pos, &mPositionList[vertA][0], posAttribBytes);
        memcpy(&currDataB.pos, &mPositionList[vertB][0], posAttribBytes);
        memcpy(&currDataC.pos, &mPositionList[vertC][0], posAttribBytes);

        memcpy(&currDataA.norm, &mNormalList[normA][0], normAttribBytes);
        memcpy(&currDataB.norm, &mNormalList[normB][0], normAttribBytes);
        memcpy(&currDataC.norm, &mNormalList[normC][0], normAttribBytes);

        memcpy(&currDataA.texcoord, &mTexcoordList[texA][0], texcoordAttribBytes);
        memcpy(&currDataB.texcoord, &mTexcoordList[texB][0], texcoordAttribBytes);
        memcpy(&currDataC.texcoord, &mTexcoordList[texC][0], texcoordAttribBytes);

    }

    unsigned int actualIndexMapIndex = 0;
    for (auto it : vertexDataMap) {
        indexDataMap[it.first] = actualIndexMapIndex;
        actualIndexMapIndex++;
        vertexData.push_back(it.second);
    }

    for (unsigned int i = 0; i < (unsigned int) mIndices.size(); i++) {
        unsigned int vertA = mIndices[i][0] - 1;
        unsigned int vertB = mIndices[i][3] - 1;
        unsigned int vertC = mIndices[i][6] - 1;

        unsigned int normA = mIndices[i][2] - 1;
        unsigned int normB = mIndices[i][5] - 1;
        unsigned int normC = mIndices[i][8] - 1;

        unsigned int texA = mIndices[i][1] - 1;
        unsigned int texB = mIndices[i][4] - 1;
        unsigned int texC = mIndices[i][7] - 1;

        VertexKey keyA = {vertA, normA, texA};
        VertexKey keyB = {vertB, normB, texB};
        VertexKey keyC = {vertC, normC, texC};

        indexData.push_back(indexDataMap[keyA]);
        indexData.push_back(indexDataMap[keyB]);
        indexData.push_back(indexDataMap[keyC]);
    }
}

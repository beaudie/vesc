
#include "SampleApplication.h"

#include <cstring>

#include <unordered_map>

#include "util/shader_utils.h"

#include "angle_capture_frame000.cpp"
#include "angle_capture_frame001.cpp"
#include "angle_capture_frame002.cpp"
#include "angle_capture_frame003.cpp"
#include "angle_capture_frame004.cpp"
#include "angle_capture_frame005.cpp"
#include "angle_capture_frame006.cpp"
#include "angle_capture_frame007.cpp"
#include "angle_capture_frame008.cpp"
#include "angle_capture_frame009.cpp"
#include "angle_capture_frame010.cpp"
#include "angle_capture_frame011.cpp"
#include "angle_capture_frame012.cpp"
#include "angle_capture_frame013.cpp"
#include "angle_capture_frame014.cpp"
#include "angle_capture_frame015.cpp"
#include "angle_capture_frame016.cpp"
#include "angle_capture_frame017.cpp"
#include "angle_capture_frame018.cpp"
#include "angle_capture_frame019.cpp"
#include "angle_capture_frame020.cpp"
#include "angle_capture_frame021.cpp"
#include "angle_capture_frame022.cpp"
#include "angle_capture_frame023.cpp"
#include "angle_capture_frame024.cpp"
#include "angle_capture_frame025.cpp"
#include "angle_capture_frame026.cpp"
#include "angle_capture_frame027.cpp"
#include "angle_capture_frame028.cpp"
#include "angle_capture_frame029.cpp"
#include "angle_capture_frame030.cpp"
#include "angle_capture_frame031.cpp"
#include "angle_capture_frame032.cpp"
#include "angle_capture_frame033.cpp"
#include "angle_capture_frame034.cpp"
#include "angle_capture_frame035.cpp"
#include "angle_capture_frame036.cpp"
#include "angle_capture_frame037.cpp"
#include "angle_capture_frame038.cpp"
#include "angle_capture_frame039.cpp"
#include "angle_capture_frame040.cpp"
#include "angle_capture_frame041.cpp"
#include "angle_capture_frame042.cpp"
#include "angle_capture_frame043.cpp"
#include "angle_capture_frame044.cpp"
#include "angle_capture_frame045.cpp"
#include "angle_capture_frame046.cpp"
#include "angle_capture_frame047.cpp"
#include "angle_capture_frame048.cpp"
#include "angle_capture_frame049.cpp"
#include "angle_capture_frame050.cpp"
#include "angle_capture_frame051.cpp"
#include "angle_capture_frame052.cpp"
#include "angle_capture_frame053.cpp"
#include "angle_capture_frame054.cpp"
#include "angle_capture_frame055.cpp"
#include "angle_capture_frame056.cpp"
#include "angle_capture_frame057.cpp"
#include "angle_capture_frame058.cpp"
#include "angle_capture_frame059.cpp"
#include "angle_capture_frame060.cpp"
#include "angle_capture_frame061.cpp"
#include "angle_capture_frame062.cpp"
#include "angle_capture_frame063.cpp"
#include "angle_capture_frame064.cpp"
#include "angle_capture_frame065.cpp"
#include "angle_capture_frame066.cpp"
#include "angle_capture_frame067.cpp"
#include "angle_capture_frame068.cpp"
#include "angle_capture_frame069.cpp"
#include "angle_capture_frame070.cpp"
#include "angle_capture_frame071.cpp"
#include "angle_capture_frame072.cpp"
#include "angle_capture_frame073.cpp"
#include "angle_capture_frame074.cpp"
#include "angle_capture_frame075.cpp"
#include "angle_capture_frame076.cpp"
#include "angle_capture_frame077.cpp"
#include "angle_capture_frame078.cpp"
#include "angle_capture_frame079.cpp"
#include "angle_capture_frame080.cpp"
#include "angle_capture_frame081.cpp"
#include "angle_capture_frame082.cpp"
#include "angle_capture_frame083.cpp"
#include "angle_capture_frame084.cpp"
#include "angle_capture_frame085.cpp"
#include "angle_capture_frame086.cpp"
#include "angle_capture_frame087.cpp"
#include "angle_capture_frame088.cpp"
#include "angle_capture_frame089.cpp"
#include "angle_capture_frame090.cpp"
#include "angle_capture_frame091.cpp"
#include "angle_capture_frame092.cpp"
#include "angle_capture_frame093.cpp"
#include "angle_capture_frame094.cpp"
#include "angle_capture_frame095.cpp"
#include "angle_capture_frame096.cpp"
#include "angle_capture_frame097.cpp"
#include "angle_capture_frame098.cpp"
#include "angle_capture_frame099.cpp"

class CaptureReplay : public SampleApplication
{
  public:
    CaptureReplay(int argc, char **argv) : SampleApplication("CatureReplay", argc, argv) {}

    bool initialize() override { return true; }

    void destroy() override {}

    void draw() override
    {
        ReplayFrame0 ReplayFrame1 ReplayFrame2 ReplayFrame3 ReplayFrame4 ReplayFrame5 ReplayFrame6 ReplayFrame7
            ReplayFrame8 ReplayFrame9 ReplayFrame10 ReplayFrame11 ReplayFrame12 ReplayFrame13 ReplayFrame14
                ReplayFrame15 ReplayFrame16 ReplayFrame17 ReplayFrame18 ReplayFrame19 ReplayFrame20 ReplayFrame21
                    ReplayFrame22 ReplayFrame23 ReplayFrame24 ReplayFrame25 ReplayFrame26 ReplayFrame27 ReplayFrame28
                        ReplayFrame29 ReplayFrame30 ReplayFrame31 ReplayFrame32 ReplayFrame33 ReplayFrame34
                            ReplayFrame35 ReplayFrame36 ReplayFrame37 ReplayFrame38 ReplayFrame39 ReplayFrame40
                                ReplayFrame41 ReplayFrame42 ReplayFrame43 ReplayFrame44 ReplayFrame45 ReplayFrame46
                                    ReplayFrame47 ReplayFrame48 ReplayFrame49 ReplayFrame50 ReplayFrame51
                                        ReplayFrame52 ReplayFrame53 ReplayFrame54 ReplayFrame55 ReplayFrame56
                                            ReplayFrame57 ReplayFrame58 ReplayFrame59 ReplayFrame60 ReplayFrame61
                                                ReplayFrame62 ReplayFrame63 ReplayFrame64 ReplayFrame65 ReplayFrame66
                                                    ReplayFrame67 ReplayFrame68 ReplayFrame69 ReplayFrame70
                                                        ReplayFrame71 ReplayFrame72 ReplayFrame73 ReplayFrame74
                                                            ReplayFrame75 ReplayFrame76 ReplayFrame77 ReplayFrame78
                                                                ReplayFrame79 ReplayFrame80 ReplayFrame81
                                                                    ReplayFrame82 ReplayFrame83 ReplayFrame84
                                                                        ReplayFrame85 ReplayFrame86 ReplayFrame87
                                                                            ReplayFrame88 ReplayFrame89 ReplayFrame90
                                                                                ReplayFrame91 ReplayFrame92
                                                                                    ReplayFrame93 ReplayFrame94
                                                                                        ReplayFrame95 ReplayFrame96
                                                                                            ReplayFrame97
                                                                                                ReplayFrame98
                                                                                                    ReplayFrame99
    }

  private:
    std::vector<uint8_t> gBinaryData;
    std::vector<uint8_t> gClientArrays[gl::MAX_VERTEX_ATTRIBS];
    std::unordered_map<GLuint, GLuint> gRenderbufferMap;

    void UpdateClientArrayPointer(int arrayIndex, const void *data, GLuint64 size)
    {
        memcpy(gClientArrays[arrayIndex].data(), data, size);
    }

    void UpdateRenderbufferID(GLuint id, GLsizei readBufferOffset)
    {
        GLuint returnedID;
        memcpy(&returnedID, &gReadBuffer[readBufferOffset], sizeof(GLuint));
        gRenderbufferMap[id] = returnedID;
    }

    void LoadBinaryData(const std::string &fname, size_t binaryDataSize)
    {
        gBinaryData.resize(static_cast<int>(binaryDataSize));
        FILE *fp = fopen(fname.c_str(), "rb");
        fread(gBinaryData.data(), 1, static_cast<int>(binaryDataSize), fp);
        fclose(fp);
    }
};

int main(int argc, char **argv)
{
    CaptureReplay app(argc, argv);
    return app.run();
}

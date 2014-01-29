#ifndef ASDFASDFASDF
#define ASDFASDFASDF

#include "common/angleutils.h"

namespace sh
{
struct ShaderVariable;
}

namespace rx
{
class Renderer;
}

namespace gl
{

class InfoLog;
class FragmentShader;
class VertexShader;
struct VariableLocation;

class DynamicHLSL
{
  public:
    explicit DynamicHLSL(rx::Renderer *const renderer);

    int packVaryings(InfoLog &infoLog, const sh::ShaderVariable *packing[][4], FragmentShader *fragmentShader);
    bool generateShaderLinkHLSL(InfoLog &infoLog, int registers, const sh::ShaderVariable *packing[][4],
                                std::string& pixelHLSL, std::string& vertexHLSL,
                                FragmentShader *fragmentShader, VertexShader *vertexShader,
                                std::map<int, VariableLocation> *programOutputVars) const;

    std::string generateGeometryShaderHLSL(int registers, const sh::ShaderVariable *packing[][4], FragmentShader *fragmentShader, VertexShader *vertexShader) const;

  private:
    DISALLOW_COPY_AND_ASSIGN(DynamicHLSL);

    rx::Renderer *const mRenderer;

    std::string generateVaryingHLSL(FragmentShader *fragmentShader, const std::string &varyingSemantic) const;
    void defineOutputVariables(FragmentShader *fragmentShader, std::map<int, VariableLocation> *programOutputVars) const;
    std::string generatePointSpriteHLSL(int registers, const sh::ShaderVariable *packing[][4], FragmentShader *fragmentShader, VertexShader *vertexShader) const;

    // Prepend an underscore
    static std::string decorateAttribute(const std::string &name);
};

// Utility method shared between ProgramBinary and DynamicHLSL
std::string ArrayString(unsigned int i);

}

#endif
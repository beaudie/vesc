//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//


#include "SampleApplication.h"
#include "shader_utils.h"

#include <GLES3/gl31.h>

class ComputeShadersample : public SampleApplication
{
  public:
    ComputeShadersample()
        : SampleApplication("ComputeShader", 1280, 720, 3, 1)
    {
    }

    virtual bool initialize()
    {
        const std::string cs = SHADER_SOURCE
        (#version 310 es\n
            layout(local_size_x=8, local_size_y=8) in;
            shared vec4 myShared[100];
            void main()
            {
                myShared[gl_LocalInvocationID.x] = vec4(1.0);
            }
        );

        mComputeProgram = CompileProgram(cs);
        if (!mComputeProgram)
        {
            return false;
        }
        
        const std::string vs = SHADER_SOURCE
        (#version 310 es\n
            precision mediump float;
            void main()
            {
              
            }
        );

        return true;
    }

    virtual void destroy()
    {
        glDeleteProgram(mComputeProgram);
        glDeleteProgram(mDisplayProgram);
    }

    virtual void draw()
    {
        // Use the program object
        glUseProgram(mComputeProgram);
	      glDispatchCompute(1,1,1);
    }

  private:
    GLuint mComputeProgram;
    GLuint mDisplayProgram;
    
    GLuint mTexture;
};

int main(int argc, char **argv)
{
    ComputeShadersample app;
    return app.run();
}

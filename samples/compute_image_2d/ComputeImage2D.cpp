//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//


#include "SampleApplication.h"
#include "shader_utils.h"

#include <GLES3/gl31.h>
#include <vector>
#include <array>
#include <cstdlib>
#include <cmath>
#include <algorithm>

class ComputeShadersample : public SampleApplication
{
  public:
    ComputeShadersample()
        : SampleApplication("ComputeShader", 1280, 720, 3, 1),
          mComputeProgram(0),
          mDisplayProgram(0),
          mTexture{0},
          mTextureX(140), // has to be multiple of 20
          mTextureY(140), // has to be multiple of 20
          mParticleVAO(0),
          mParticleVBO(0),
          mNumParticles(mTextureX*mTextureY)
    {
    }

    virtual bool initialize()
    {
        const std::string cs = SHADER_SOURCE
        (#version 310 es\n
            
            
            layout (local_size_x = 20, local_size_y = 20, local_size_z = 1) in;
            
            precision mediump image2D;
            
            // positions from the previous frame
            layout(rgba32f, binding = 1) uniform readonly image2D positionImage;
            
            // positions from two frames ago
            // we will use this one to store the new positions
            layout(rgba32f, binding = 2) uniform image2D positionImageResult;
            
            // the texture size
            uniform ivec2 imageSize;
            
            // verlet integration function
            vec3 verlet(in vec3 a, in vec3 x, in vec3 xOld, in float dt) {
                return 2.0 * x - xOld + a * (dt*dt);
            }
            
            // allocate enough memory to do shared memory blocking
            shared vec4 sharedPositions[400];
            
            void main()
            {
                const float G = 9.8;
                
                // check the index and see whether we are going out of bounds of the image
                ivec2 globalID = ivec2(gl_GlobalInvocationID.xy);
                if (globalID.x >= imageSize.x || globalID.y >= imageSize.y) return;
                
                ivec2 localID = ivec2(gl_LocalInvocationID.xy);
                
                // particle information
                vec4 particleInfo = imageLoad(positionImage, globalID);
                vec3 pos = particleInfo.xyz;
                float mass = particleInfo.w;
                
                // get the position of the particle two frames ago
                vec3 oldPos = imageLoad(positionImageResult, globalID).xyz;
                
                // variable to accumulate acceleration
                vec3 acc = vec3(0.0);
                
                // go over all particles since we are doing the O(N^2) NBody simmulation
                // iterate over the image in blocks of 20x20
                for (int i = 0; i < imageSize.x; i+=20) {
                  for (int j = 0; j < imageSize.y; j+=20) {
                    
                    // each thread in the local group loads an entry and stores it into shared memory
                    sharedPositions[localID.y * 20 + localID.x] = imageLoad(positionImage, localID+ivec2(i,j));
                    
                    // we have to sync the threads and the shared memory
                    barrier();
                    memoryBarrierShared();
                    
                    // at this point all of the threads in the local group can access the shared memory
                    
                    // go over the cached data
                    for (int ii = 0; ii < 20; ++ii) {
                      for (int jj = 0; jj < 20; ++jj) {
                        
                        // get the other particle's position and mass
                        vec4 otherParticleInfo = sharedPositions[jj + ii*20];
                        vec3 otherPos = otherParticleInfo.xyz;
                        float otherMass = otherParticleInfo.w;
                        
                        // get direction towards the other particle.
                        // the current particle gets attracted by in that direction
                        vec3 dirToParticle = (otherPos-pos);
                        
                        // compute the squared distance by just taking the dot product
                        float rSq = dot(dirToParticle,dirToParticle);
                        
                        // compute the acceleration
                        // acc = G * M * dir / (distance + eps)^(3/2)
                        acc += dirToParticle * otherMass / pow(rSq + 0.1, 1.5);
                      }
                    }
                    
                    // sync all of the threads
                    // this is required since we do not want some thread to go and update the shared memory
                    barrier();

                  }
                  
                }
                
                acc *= G;
                
                // integrate over the vector field
                // verlet integration is generally unstable, but we have to bite the bullet
                // and use it since it is also very efficient
                vec3 newPos = verlet(acc, pos, oldPos, 0.0001);
                
                // update the image
                imageStore(positionImageResult, globalID, vec4(newPos,mass));
            }
        );

        mComputeProgram = CompileComputeProgram(cs);
        if (!mComputeProgram)
        {
            return false;
        }
        
        const std::string vs = SHADER_SOURCE
        (#version 310 es\n
            precision mediump float;
            precision mediump image2D;
            
            // the coordinate of the particle in the image
            in ivec2 pointImageIndex;
            
            // the image from which to read the particle's position
            layout(rgba32f) uniform readonly image2D particleImage;
            
            void main()
            {
                // get the particles position
                vec4 particleData = imageLoad(particleImage, pointImageIndex);
                
                gl_Position = vec4(particleData.xy, 0.0, 1.0);
            }
        );
        
        const std::string fs = SHADER_SOURCE
        (#version 310 es\n
            precision mediump float;
            
            out vec4 color;
            
            void main()
            {
                color = vec4(1.0);
            }
        );
        
        mDisplayProgram = CompileProgram(vs, fs);
        if (!mDisplayProgram)
        {
            return false;
        }
        
        generateParticles();

        return true;
    }

    virtual void destroy()
    {
        glDeleteProgram(mComputeProgram);
        glDeleteProgram(mDisplayProgram);
    }

    virtual void draw()
    {
        updateParticles();
        
        glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());
        
        glClearColor(0,0,0,1);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        
        glUseProgram(mDisplayProgram);
        
        // bind the image so that we can read the position of the particles in the program
        glBindImageTexture(0, mTexture[0], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
        
        glBindVertexArray(mParticleVAO);
        glDrawArrays(GL_POINTS, 0, mNumParticles);
        glBindVertexArray(0);
        
        glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    }
    
    void updateParticles()
    {
        
        // The local group size is 20. It can vary from gpu to gpu. You have to try and see whether you can
        // get better results with a smaller or bigger gorup size
        static const Vec2i localSize = {20,20};
        static const Vec2i totalSize = {int(mTextureX), int(mTextureY)};
        // compute how many groups we have to dispatch
        static const Vec2i numGroups = {(totalSize[0] + localSize[0] - 1) / localSize[0], (totalSize[1] + localSize[1] - 1) / localSize[1]};
      
        glUseProgram(mComputeProgram);
        
        glUniform2i(glGetUniformLocation(mComputeProgram, "imageSize"), totalSize[0], totalSize[1]);
        
        // bind the images we will be using
        glBindImageTexture(1, mTexture[0], 0, false, 0, GL_READ_ONLY, GL_RGBA32F);
		    glBindImageTexture(2, mTexture[1], 0, false, 0, GL_READ_WRITE, GL_RGBA32F);

        glDispatchCompute(numGroups[0], numGroups[1], 1);
        
        glBindImageTexture(1, 0, 0, false, 0, GL_READ_ONLY, GL_RGBA32F);
		    glBindImageTexture(2, 0, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
        
        // we need to synchronize texture memory otherwise changes might not be visible
    		glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
        
        // initially texture 0 has the state from the previous frame and texture 1 has the state from two frames ago
        // after updating the particles texture 1 has the state for the current frame
        // we have to swap the particles
    		std::swap(mTexture[0], mTexture[1]);
    }
    
    void generateParticles()
    {
        static const float RADIUS = 1.0f;
        static const float KPI = 3.14159265358979323846f;
        std::vector<Vec4f> particleData(mNumParticles);

        srand(10100111);
        unsigned k = 0;
        
        // generate random particles on the surface of a sphere
        for (unsigned x = 0; x < mTextureX; ++x)
        {
            for (unsigned y = 0; y < mTextureY; ++y)
            {
                // get random number in [0, 2*PI] for x and, [0, PI] for y
                float rndX = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
                float rndY = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
                rndX *= 2.0f * KPI;
                rndY *= KPI;
                
                // convert from spherical to cartesian coordinates
                float pointX = RADIUS * cosf(rndX) * sinf(rndY);
        				float pointY = RADIUS * sinf(rndX) * sinf(rndY);
        				float pointZ = RADIUS * cosf(rndY);
                
                float pointMass = 0.25f;
                
                // have one point with very high mass
                if (pointX < 0.0f)
                {
                    pointMass = abs(pointX)*15.0f;
                }
                
                particleData[k][0] = pointX;
                particleData[k][1] = pointY;
                particleData[k][2] = pointZ;
                particleData[k][3] = pointMass;
                ++k;
            }
        }
        
        // generate particle image indices we can use when rendering
        std::vector<Vec2i> particleIndices(mNumParticles);
        k = 0;
        for (unsigned x = 0; x < mTextureX; ++x)
        {
            for (unsigned y = 0; y < mTextureY; ++y)
            {
                particleIndices[k][0] = int(x);
                particleIndices[k][1] = int(y);
                ++k;
            }
        }
        
        // generate particle indices buffer
        glGenVertexArrays(1, &mParticleVAO);
		    glBindVertexArray(mParticleVAO);
        
        glGenBuffers(1, &mParticleVBO);

    		glBindBuffer(GL_ARRAY_BUFFER, mParticleVBO);

    		glBufferData(GL_ARRAY_BUFFER, sizeof(Vec2i) *mNumParticles, particleIndices.data(), GL_STATIC_DRAW);

    		glEnableVertexAttribArray(0);
    		glVertexAttribIPointer(0, 2, GL_INT, 0, 0);

    		glBindBuffer(GL_ARRAY_BUFFER, 0);
    		glBindVertexArray(0);
        
        // generate textures
        glGenTextures(2, mTexture);

    		glBindTexture(GL_TEXTURE_2D, mTexture[0]);
    		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, mTextureX, mTextureY);
    		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mTextureX, mTextureX, GL_RGBA, GL_FLOAT, particleData.data());


    		glBindTexture(GL_TEXTURE_2D, mTexture[1]);
    		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, mTextureX, mTextureY);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mTextureX, mTextureY, GL_RGBA, GL_FLOAT, particleData.data());
        
    		glBindTexture(GL_TEXTURE_2D, 0);
    }

  private:
    
    typedef std::array<float, 4> Vec4f;
    typedef std::array<int, 2> Vec2i;
    
    GLuint mComputeProgram;
    GLuint mDisplayProgram;
    
    GLuint mTexture[2];
    unsigned mTextureX;
    unsigned mTextureY;
    
    GLuint mParticleVAO;
    GLuint mParticleVBO;
    unsigned mNumParticles;
};

int main(int argc, char **argv)
{
    ComputeShadersample app;
    return app.run();
}

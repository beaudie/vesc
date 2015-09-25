//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

//            Based on Hello_Triangle.c from
// Book:      OpenGL(R) ES 2.0 Programming Guide
// Authors:   Aaftab Munshi, Dan Ginsburg, Dave Shreiner
// ISBN-10:   0321502795
// ISBN-13:   9780321502797
// Publisher: Addison-Wesley Professional
// URLs:      http://safari.informit.com/9780321563835
//            http://www.opengles-book.com

#include "SampleApplication.h"
#include "shader_utils.h"

#include <iostream>

#include "counter.h"

namespace n1 {
    COUNTER1
}
namespace n2 {
    COUNTER2
}
namespace n3 {
    COUNTER3
}
namespace n4 {
    COUNTER4
}
namespace n5 {
    COUNTER5
}
namespace n6 {
    COUNTER6
}
namespace n7 {
    COUNTER7
}

int (*cCounters[])() = {
    n1::counter,
    n2::counter,
    n3::counter,
    n4::counter,
    n5::counter,
    n6::counter,
    n7::counter,
};

class HelloTriangleSample : public SampleApplication
{
  public:
    HelloTriangleSample()
        : SampleApplication("HelloTriangle", 1280, 720)
    {
    }

    virtual bool initialize()
    {
        const char *counters[] = {
            STRINGIFY(COUNTER1),
            STRINGIFY(COUNTER2),
            STRINGIFY(COUNTER3),
            STRINGIFY(COUNTER4),
            STRINGIFY(COUNTER5),
            STRINGIFY(COUNTER6),
            STRINGIFY(COUNTER7),
        };

        const std::string vs = SHADER_SOURCE
        (
            precision mediump float;
            attribute vec4 vPosition;
            varying float vertexCounter;

            void main()
            {
                gl_Position = vPosition;
                vertexCounter = float(counter());
            }
        );

        const std::string fs = SHADER_SOURCE
        (
            precision mediump float;
            varying float vertexCounter;

            void main()
            {
                gl_FragColor = vec4(1.0, vertexCounter / 255.0 * 10.0, float(counter()) / 255.0 * 10.0, 1.0);
            }
        );

        mPrograms.resize(sizeof(counters) / sizeof(counters[0]));
        for (int i = 0; i < mPrograms.size(); i++) {
            std::string counter = counters[i];
            mPrograms[i] = CompileProgram(counter + vs, counter + fs);
        }

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        return true;
    }

    virtual void destroy()
    {
        //glDeleteProgram(mProgram);
    }

    virtual void draw()
    {
        static bool first = true;

        for (int i = 0; i < mPrograms.size(); i++) {
            GLfloat vertices[] =
            {
                 0.0f,  0.5f, 0.0f,
                -0.5f, -0.5f, 0.0f,
                 0.5f, -0.5f, 0.0f,
            };

            // Set the viewport
            glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());

            // Clear the color buffer
            glClear(GL_COLOR_BUFFER_BIT);

            // Use the program object
            glUseProgram(mPrograms[i]);

            // Load the vertex data
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices);
            glEnableVertexAttribArray(0);

            glDrawArrays(GL_TRIANGLES, 0, 3);

            if (first) {
                GLubyte pixel[4];
                glReadPixels(640, 360, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);

                std::cout << "=== Counter " << i + 1 << " ===" << std::endl;
                std::cout << "C: " << cCounters[i]() * 10 << std::endl;
                std::cout << "V: " << (int) pixel[1] << std::endl;
                std::cout << "F: " << (int) pixel[2] << std::endl;
            }
        }

        first = false;
    }

  private:
    std::vector<GLuint> mPrograms;
};

int main(int argc, char **argv)
{
    HelloTriangleSample app;
    return app.run();
}

#include "ANGLETest.h"
#include "EGLWindow.h"
#include "OSWindow.h"

EGLWindow *ANGLETest::mEGLWindow = nullptr;
OSWindow *ANGLETest::mOSWindow = nullptr;

namespace
{

bool ShouldRecreateEGL(EGLint glesMajorVersion, const EGLPlatformParameters &platform, EGLWindow *eglWindow)
{
    if (static_cast<EGLint>(eglWindow->getClientVersion()) != glesMajorVersion)
    {
        return true;
    }

    if (eglWindow->getPlatform().deviceType != platform.deviceType)
    {
        return true;
    }

    if (eglWindow->getPlatform().majorVersion != platform.majorVersion)
    {
        return true;
    }

    if (eglWindow->getPlatform().minorVersion != platform.minorVersion)
    {
        return true;
    }

    if (eglWindow->getPlatform().renderer != platform.renderer)
    {
        return true;
    }

    return false;
}

}

ANGLETest::ANGLETest(EGLint glesMajorVersion, const EGLPlatformParameters &platform)
    : mEGLWindowNeedsReinit(false)
{
    if (mEGLWindow != nullptr && ShouldRecreateEGL(glesMajorVersion, platform, mEGLWindow))
    {
        delete mEGLWindow;
        mEGLWindow = nullptr;
        mEGLWindowNeedsReinit = true;
    }

    if (mEGLWindow == nullptr)
    {
        mEGLWindow = new EGLWindow(1280, 720, glesMajorVersion, platform);
        mEGLWindowNeedsReinit = true;
    }
}

ANGLETest::~ANGLETest()
{
}

void ANGLETest::SetUp()
{
    if (!mEGLWindowNeedsReinit)
    {
        resetState();
        return;
    }

    if (!destroyEGLContext())
    {
        FAIL() << "egl context destruction failed.";
    }

    if (!ResizeWindow(mEGLWindow->getWidth(), mEGLWindow->getHeight()))
    {
        FAIL() << "Failed to resize ANGLE test window.";
    }

    if (!createEGLContext())
    {
        FAIL() << "egl context creation failed.";
    }

    mEGLWindowNeedsReinit = false;
}

void ANGLETest::TearDown()
{
    swapBuffers();
    mOSWindow->messageLoop();

    // Check for quit message
    Event myEvent;
    while (mOSWindow->popEvent(&myEvent))
    {
        if (myEvent.Type == Event::EVENT_CLOSED)
        {
            exit(0);
        }
    }
}

void ANGLETest::swapBuffers()
{
    mEGLWindow->swap();
}

void ANGLETest::drawQuad(GLuint program, const std::string& positionAttribName, GLfloat quadDepth, GLfloat quadScale)
{
    GLint positionLocation = glGetAttribLocation(program, positionAttribName.c_str());

    glUseProgram(program);

    const GLfloat vertices[] =
    {
        -1.0f * quadScale,  1.0f * quadScale, quadDepth,
        -1.0f * quadScale, -1.0f * quadScale, quadDepth,
         1.0f * quadScale, -1.0f * quadScale, quadDepth,

        -1.0f * quadScale,  1.0f * quadScale, quadDepth,
         1.0f * quadScale, -1.0f * quadScale, quadDepth,
         1.0f * quadScale,  1.0f * quadScale, quadDepth,
    };

    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(positionLocation);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(positionLocation);
    glVertexAttribPointer(positionLocation, 4, GL_FLOAT, GL_FALSE, 0, NULL);

    glUseProgram(0);
}

GLuint ANGLETest::compileShader(GLenum type, const std::string &source)
{
    GLuint shader = glCreateShader(type);

    const char *sourceArray[1] = { source.c_str() };
    glShaderSource(shader, 1, sourceArray, NULL);
    glCompileShader(shader);

    GLint compileResult;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResult);

    if (compileResult == 0)
    {
        GLint infoLogLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

        std::vector<GLchar> infoLog(infoLogLength);
        glGetShaderInfoLog(shader, infoLog.size(), NULL, &infoLog[0]);

        std::cerr << "shader compilation failed: " << &infoLog[0];

        glDeleteShader(shader);
        shader = 0;
    }

    return shader;
}

bool ANGLETest::extensionEnabled(const std::string &extName)
{
    const char* extString = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
    return strstr(extString, extName.c_str()) != NULL;
}

void ANGLETest::setWindowWidth(int width)
{
    if (width != mEGLWindow->getWidth())
    {
        mEGLWindow->setWidth(width);
        mEGLWindowNeedsReinit = true;
    }
}

void ANGLETest::setWindowHeight(int height)
{
    if (height != mEGLWindow->getHeight())
    {
        mEGLWindow->setHeight(height);
        mEGLWindowNeedsReinit = true;
    }
}

void ANGLETest::setConfigRedBits(int bits)
{
    if (bits != mEGLWindow->getConfigRedBits())
    {
        mEGLWindow->setConfigRedBits(bits);
        mEGLWindowNeedsReinit = true;
    }
}

void ANGLETest::setConfigGreenBits(int bits)
{
    if (bits != mEGLWindow->getConfigGreenBits())
    {
        mEGLWindow->setConfigGreenBits(bits);
        mEGLWindowNeedsReinit = true;
    }
}

void ANGLETest::setConfigBlueBits(int bits)
{
    if (bits != mEGLWindow->getConfigBlueBits())
    {
        mEGLWindow->setConfigBlueBits(bits);
        mEGLWindowNeedsReinit = true;
    }
}

void ANGLETest::setConfigAlphaBits(int bits)
{
    if (bits != mEGLWindow->getConfigAlphaBits())
    {
        mEGLWindow->setConfigAlphaBits(bits);
        mEGLWindowNeedsReinit = true;
    }
}

void ANGLETest::setConfigDepthBits(int bits)
{
    if (bits != mEGLWindow->getConfigDepthBits())
    {
        mEGLWindow->setConfigDepthBits(bits);
        mEGLWindowNeedsReinit = true;
    }
}

void ANGLETest::setConfigStencilBits(int bits)
{
    if (bits != mEGLWindow->getConfigStencilBits())
    {
        mEGLWindow->setConfigStencilBits(bits);
        mEGLWindowNeedsReinit = true;
    }
}

void ANGLETest::setMultisampleEnabled(bool enabled)
{
    if (enabled != mEGLWindow->isMultisample())
    {
        mEGLWindow->setMultisample(enabled);
        mEGLWindowNeedsReinit = true;
    }
}

int ANGLETest::getClientVersion() const
{
    return mEGLWindow->getClientVersion();
}

EGLWindow *ANGLETest::getEGLWindow() const
{
    return mEGLWindow;
}

int ANGLETest::getWindowWidth() const
{
    return mEGLWindow->getWidth();
}

int ANGLETest::getWindowHeight() const
{
    return mEGLWindow->getHeight();
}

bool ANGLETest::isMultisampleEnabled() const
{
    return mEGLWindow->isMultisample();
}

bool ANGLETest::createEGLContext()
{
    return mEGLWindow->initializeGL(mOSWindow);
}

bool ANGLETest::destroyEGLContext()
{
    mEGLWindow->destroyGL();
    return true;
}

bool ANGLETest::InitTestWindow()
{
    mOSWindow = CreateOSWindow();
    if (!mOSWindow->initialize("ANGLE_TEST", 128, 128))
    {
        return false;
    }

    mOSWindow->setVisible(true);

    return true;
}

bool ANGLETest::DestroyTestWindow()
{
    if (mOSWindow)
    {
        mOSWindow->destroy();
        delete mOSWindow;
        mOSWindow = nullptr;
    }

    if (mEGLWindow)
    {
        delete mEGLWindow;
        mEGLWindow = nullptr;
    }

    return true;
}

bool ANGLETest::ResizeWindow(int width, int height)
{
    return mOSWindow->resize(width, height);
}

void ANGLETest::SetWindowVisible(bool isVisible)
{
    mOSWindow->setVisible(isVisible);
}

bool ANGLETest::isIntel() const
{
    std::string rendererString(reinterpret_cast<const char *>(glGetString(GL_RENDERER)));
    return (rendererString.find("Intel") != std::string::npos);
}

bool ANGLETest::isAMD() const
{
    std::string rendererString(reinterpret_cast<const char *>(glGetString(GL_RENDERER)));
    return (rendererString.find("AMD") != std::string::npos) ||
           (rendererString.find("ATI") != std::string::npos);
}

bool ANGLETest::isNVidia() const
{
    std::string rendererString(reinterpret_cast<const char *>(glGetString(GL_RENDERER)));
    return (rendererString.find("NVIDIA") != std::string::npos);
}

EGLint ANGLETest::getPlatformRenderer() const
{
    assert(mEGLWindow);
    return mEGLWindow->getPlatform().renderer;
}

void ANGLETestEnvironment::SetUp()
{
    if (!ANGLETest::InitTestWindow())
    {
        FAIL() << "Failed to create ANGLE test window.";
    }
}

void ANGLETestEnvironment::TearDown()
{
    ANGLETest::DestroyTestWindow();
}

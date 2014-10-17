//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "SampleApplication.h"
#include "Font.h"
#include "random_utils.h"
#include "path_utils.h"

class SimpleTextSample : public SampleApplication
{
  public:
      SimpleTextSample::SimpleTextSample()
        : SampleApplication("SimpleText", 1280, 720)
    {
    }

    virtual bool initialize()
    {
        if (!mFontRenderer.initialize())
        {
            return false;
        }

        if (!LoadTTFFontFromFile(GetExecutableDirectory() + "/sample_font.ttf", &mFont))
        {
            return false;
        }

        mFont.prepareText("The quick brown fox\njumps over the\nlazy dog", 72, &mTextSize, &mPreparedGlyphs);

        mTextPosition = Vector2(RandomBetween(0.0f, getWindow()->getWidth() - mTextSize.x),
                                RandomBetween(mTextSize.y, getWindow()->getHeight()));

        mTextVelocity = Vector2(250.0f, 250.0f);

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        return true;
    }

    virtual void destroy()
    {
    }

    virtual void step(float dt, double totalTime)
    {
        if ((mTextPosition.x < 0.0f && mTextVelocity.x < 0) ||
            (mTextPosition.x >(getWindow()->getWidth() - mTextSize.x) && mTextVelocity.x > 0))
        {
            mTextVelocity.x = -mTextVelocity.x;
        }
        if ((mTextPosition.y < mTextSize.y && mTextVelocity.y < 0) ||
            (mTextPosition.y > getWindow()->getHeight() && mTextVelocity.y > 0))
        {
            mTextVelocity.y = -mTextVelocity.y;
        }
        mTextPosition = mTextPosition + (mTextVelocity * dt);
    }

    virtual void draw()
    {
        // Clear the color buffer
        glClear(GL_COLOR_BUFFER_BIT);

        mFontRenderer.renderText(mPreparedGlyphs, mTextPosition, Vector4(1, 1, 1, 1));
    }

  private:
    Font mFont;
    FontRenderer mFontRenderer;

    std::vector<Glyph> mPreparedGlyphs;
    Vector2 mTextSize;

    Vector2 mTextPosition;
    Vector2 mTextVelocity;
};

int main(int argc, char **argv)
{
    SimpleTextSample app;
    return app.run();
}

//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef LIBANGLE_ATTRIBUTEMAP_H_
#define LIBANGLE_ATTRIBUTEMAP_H_


#include <EGL/egl.h>

#include <map>

namespace egl
{

template <typename T>
class AttributeMapBase final
{
  public:
    AttributeMapBase() {}
    explicit AttributeMapBase(const T *attributes)
    {
        if (attributes)
        {
            for (const T *curAttrib = attributes; curAttrib[0] != EGL_NONE; curAttrib += 2)
            {
                insert(curAttrib[0], curAttrib[1]);
            }
        }
    }

    void insert(T key, T value) { mAttributes[key] = value; }

    bool contains(T key) const { return (mAttributes.find(key) != mAttributes.end()); }

    T get(T key, T defaultValue) const
    {
        std::map<T, T>::const_iterator iter = mAttributes.find(key);
        return (mAttributes.find(key) != mAttributes.end()) ? iter->second : defaultValue;
    }

    typedef typename std::map<T, T>::const_iterator const_iterator;

    typename std::map<T, T>::const_iterator begin() const { return mAttributes.begin(); }

    typename std::map<T, T>::const_iterator end() const { return mAttributes.end(); }

  private:
    std::map<T, T> mAttributes;
};

typedef AttributeMapBase<EGLint> AttributeMap;
typedef AttributeMapBase<EGLAttrib> AttributeMapEGL1_5;
}

#endif   // LIBANGLE_ATTRIBUTEMAP_H_

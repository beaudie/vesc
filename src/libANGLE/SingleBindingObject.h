//
// Copyright (c) 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SingleBindingObject.h: A SingleBindingObject can be bound to one
// SingleBindingPointer at a time. It is notified whenever it is bound or
// unbound. This is used for VAOs and transform feedback objects so they can
// notify their buffers of bind and unbind events.

#ifndef LIBANGLE_SINGLEBINDINGOBJECT_H_
#define LIBANGLE_SINGLEBINDINGOBJECT_H_

#include <cstddef>
#include "angle_gl.h"
#include "common/debug.h"
#include "libANGLE/Error.h"
#include "libANGLE/RefCountObject.h"

namespace gl
{
class Context;

class SingleBindingObject : angle::NonCopyable
{
  public:
    SingleBindingObject() : mIsBound(false) {}

  protected:
    template <class ObjectType>
    friend class SingleBindingPointer;

    virtual void onBindingChange() {}
    bool mIsBound;
};

template <class ObjectType>
class SingleBindingPointer : public BindingPointer<ObjectType>, angle::NonCopyable
{
  public:
    SingleBindingPointer() {}

    SingleBindingPointer(ObjectType *object) { set(object); }

    void set(const Context *context, ObjectType *newObject) override
    {
        if (BindingPointer<ObjectType>::get())
        {
            ASSERT(BindingPointer<ObjectType>::get()->mIsBound);
            BindingPointer<ObjectType>::get()->mIsBound = false;
            static_cast<SingleBindingObject *>(BindingPointer<ObjectType>::get())
                ->onBindingChange();
        }
        BindingPointer<ObjectType>::set(context, newObject);
        if (newObject)
        {
            ASSERT(!newObject->mIsBound);
            if (!newObject->mIsBound)
            {
                newObject->mIsBound = true;
                static_cast<SingleBindingObject *>(newObject)->onBindingChange();
            }
        }
    }
};

}  // namespace gl

#endif  // LIBANGLE_SINGLEBINDINGOBJECT_H_

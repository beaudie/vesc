//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Observer:
//   Implements the Observer pattern for sending state change notifications
//   from Subject objects to dependent Observer objects.
//
//   See design document:
//   https://docs.google.com/document/d/15Edfotqg6_l1skTEL8ADQudF_oIdNa7i8Po43k6jMd4/

#ifndef LIBANGLE_OBSERVER_H_
#define LIBANGLE_OBSERVER_H_

#include "common/ListNode.h"
#include "common/debug.h"

namespace angle
{
using SubjectIndex = size_t;

// Messages are used to distinguish different Subject events that get sent to a single Observer.
// It could be possible to improve the handling by using different callback functions instead
// of a single handler function. But in some cases we want to share a single binding between
// Observer and Subject and handle different types of events.
enum class SubjectMessage
{
    // Used by gl::VertexArray to notify gl::Context of a gl::Buffer binding count change. Triggers
    // a validation cache update. Also used by gl::Texture to notify gl::Framebuffer of loops.
    BindingChanged,

    // Only the contents (pixels, bytes, etc) changed in this Subject. Distinct from the object
    // storage.
    ContentsChanged,

    // Sent by gl::Sampler, gl::Texture, gl::Framebuffer and others to notifiy gl::Context. This
    // flag indicates to call syncState before next use.
    DirtyBitsFlagged,

    // Generic state change message. Used in multiple places for different purposes.
    SubjectChanged,

    // Indicates a bound gl::Buffer is now mapped or unmapped. Passed from gl::Buffer, through
    // gl::VertexArray, into gl::Context. Used to track validation.
    SubjectMapped,
    SubjectUnmapped,

    // Indicates an external change to the default framebuffer.
    SurfaceChanged,
};

// The observing class inherits from this interface class.
class ObserverInterface
{
  public:
    virtual ~ObserverInterface();
    virtual void onSubjectStateChange(SubjectIndex index, SubjectMessage message) = 0;
};

class ObserverBindingBase : public angle::ListNode
{
  public:
    ObserverBindingBase(ObserverInterface *observer, SubjectIndex subjectIndex)
        : mObserver(observer), mIndex(subjectIndex)
    {}
    virtual ~ObserverBindingBase() {}

    ObserverInterface *getObserver() const { return mObserver; }
    SubjectIndex getSubjectIndex() const { return mIndex; }
    virtual void onSubjectReset() {}

  private:
    ObserverInterface *mObserver;
    SubjectIndex mIndex;
};

// Maintains a list of observer bindings. Sends update messages to the observer.
class Subject : NonCopyable
{
  public:
    Subject();
    virtual ~Subject();

    void onStateChange(SubjectMessage message) const;
    ANGLE_INLINE bool hasObservers() const { return (mSentinelNode.getNext() != &mSentinelNode); }
    void resetObservers();

    ANGLE_INLINE void addObserver(ObserverBindingBase *observer)
    {
        ASSERT(!isInContainer(observer));
        // Always add at head
        mSentinelNode.insertListNode(observer);
    }

    ANGLE_INLINE void removeObserver(ObserverBindingBase *observer)
    {
        ASSERT(isInContainer(observer));
        mSentinelNode.removeListNode(observer);
    }

  private:
    ANGLE_INLINE bool isInContainer(ObserverBindingBase *observer)
    {
        angle::ListNode *node = static_cast<angle::ListNode *>(observer);
        return (node->getNext() != node);
    }

    angle::ListNode mSentinelNode;
};

// Keeps a binding between a Subject and Observer, with a specific subject index.
class ObserverBinding final : public ObserverBindingBase
{
  public:
    ObserverBinding(ObserverInterface *observer, SubjectIndex index);
    ~ObserverBinding() override;
    ObserverBinding(const ObserverBinding &other);
    ObserverBinding &operator=(const ObserverBinding &other);

    void bind(Subject *subject);

    ANGLE_INLINE void reset() { bind(nullptr); }

    void onStateChange(SubjectMessage message) const;
    void onSubjectReset() override;

    ANGLE_INLINE const Subject *getSubject() const { return mSubject; }

    ANGLE_INLINE void assignSubject(Subject *subject) { mSubject = subject; }

  private:
    Subject *mSubject;
};

}  // namespace angle

#endif  // LIBANGLE_OBSERVER_H_

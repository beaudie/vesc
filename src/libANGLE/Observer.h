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

#include "common/angleutils.h"

namespace gl
{
class Context;
}  // namespace gl

namespace angle
{

using SubjectIndex = size_t;

enum class SubjectMessage
{
    STATE_CHANGE,
    DEPENDENT_DIRTY_BITS,
};

// The observing class inherits from this interface class.
// Note that because of MSVC, classes that inherit from ObserverInterface must specify it as the
// first base class.
class ObserverInterface
{
  public:
    virtual ~ObserverInterface();
    using Function = void (ObserverInterface::*)(const gl::Context *context,
                                                 SubjectIndex index,
                                                 SubjectMessage message);
};

class ObserverBinding;

// Maintains a list of observer bindings. Sends update messages to the observer.
class Subject : NonCopyable
{
  public:
    Subject();
    virtual ~Subject();

    void onStateChange(const gl::Context *context, SubjectMessage message) const;
    bool hasObservers() const;
    void resetObservers();

  private:
    // Only the ObserverBinding class should add or remove observers.
    friend class ObserverBinding;
    void addObserver(ObserverBinding *observer);
    void removeObserver(ObserverBinding *observer);

    std::vector<ObserverBinding *> mObservers;
};

// Keeps a binding between a Subject and Observer, with a specific subject index.
class ObserverBinding final
{
  public:
    ObserverBinding(ObserverInterface *observer, SubjectIndex index);
    ~ObserverBinding();
    ObserverBinding(const ObserverBinding &other);
    ObserverBinding &operator=(const ObserverBinding &other);

    template <typename FuncT>
    void bind(Subject *subject, FuncT function);
    void reset();
    void onStateChange(const gl::Context *context, SubjectMessage message) const;
    void onSubjectReset();

    const Subject *getSubject() const;

  private:
    void bind(Subject *subject, ObserverInterface::Function function);

    Subject *mSubject;
    ObserverInterface *mObserver;
    SubjectIndex mIndex;
    ObserverInterface::Function mFunction;
};

template <typename FuncT>
void ObserverBinding::bind(Subject *subject, FuncT function)
{
    bind(subject, static_cast<ObserverInterface::Function>(function));
}

}  // namespace angle

#endif  // LIBANGLE_OBSERVER_H_

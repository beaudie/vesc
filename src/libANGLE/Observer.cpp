//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Observer:
//   Implements the Observer pattern for sending state change notifications
//   from Subject objects to dependent Observer objects.
//
//   See design document:
//   https://docs.google.com/document/d/15Edfotqg6_l1skTEL8ADQudF_oIdNa7i8Po43k6jMd4/

#include "libANGLE/Observer.h"

#include <algorithm>

#include "common/debug.h"

namespace angle
{
namespace
{}  // anonymous namespace

// Observer implementation.
ObserverInterface::~ObserverInterface() = default;

// Subject implementation.
Subject::Subject() : mObserverBindingBaseHead(nullptr) {}

Subject::~Subject()
{
    resetObservers();
}

bool Subject::hasObservers() const
{
    return (mObserverBindingBaseHead != nullptr);
}

void Subject::onStateChange(SubjectMessage message) const
{
    if (!mObserverBindingBaseHead)
    {
        return;
    }

    ObserverBindingBase *binding = mObserverBindingBaseHead;
    while (binding)
    {
        binding->getObserver()->onSubjectStateChange(binding->getSubjectIndex(), message);
        binding = binding->getNext();
    }
}

void Subject::resetObservers()
{
    if (!mObserverBindingBaseHead)
    {
        return;
    }

    ObserverBindingBase *binding = mObserverBindingBaseHead;
    ObserverBindingBase *next    = nullptr;
    while (binding)
    {
        binding->onSubjectReset();
        next = binding->getNext();
        binding->setPrevious(nullptr);
        binding->setNext(nullptr);
        binding = next;
    }
    mObserverBindingBaseHead = nullptr;
}

// ObserverBinding implementation.
ObserverBinding::ObserverBinding(ObserverInterface *observer, SubjectIndex index)
    : ObserverBindingBase(observer, index), mSubject(nullptr)
{
    ASSERT(observer);
}

ObserverBinding::~ObserverBinding()
{
    reset();
}

ObserverBinding::ObserverBinding(const ObserverBinding &other) = default;

ObserverBinding &ObserverBinding::operator=(const ObserverBinding &other) = default;

void ObserverBinding::bind(Subject *subject)
{
    ASSERT(getObserver());
    if (mSubject)
    {
        mSubject->removeObserver(this);
    }

    mSubject = subject;

    if (mSubject)
    {
        mSubject->addObserver(this);
    }
}

void ObserverBinding::onStateChange(SubjectMessage message) const
{
    getObserver()->onSubjectStateChange(getSubjectIndex(), message);
}

void ObserverBinding::onSubjectReset()
{
    mSubject = nullptr;
}
}  // namespace angle

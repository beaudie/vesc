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

namespace angle
{
namespace
{}  // anonymous namespace

// Observer implementation.
ObserverInterface::~ObserverInterface() = default;

// Subject implementation.
Subject::Subject() {}

Subject::~Subject()
{
    resetObservers();
}

void Subject::onStateChange(SubjectMessage message) const
{
    if (!hasObservers())
    {
        return;
    }

    angle::ListNode *currentNode = mSentinelNode.getNext();
    while (currentNode != &mSentinelNode)
    {
        ObserverBindingBase *binding = static_cast<ObserverBindingBase *>(currentNode);
        binding->getObserver()->onSubjectStateChange(binding->getSubjectIndex(), message);
        currentNode = currentNode->getNext();
    }
}

void Subject::resetObservers()
{
    if (!hasObservers())
    {
        return;
    }

    angle::ListNode *currentNode = mSentinelNode.getNext();
    angle::ListNode *nextNode    = nullptr;
    while (currentNode != &mSentinelNode)
    {
        ObserverBindingBase *binding = static_cast<ObserverBindingBase *>(currentNode);
        binding->onSubjectReset();
        nextNode = currentNode->getNext();
        currentNode->detachListNode();
        currentNode = nextNode;
    }
    mSentinelNode.detachListNode();
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

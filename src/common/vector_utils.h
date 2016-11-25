//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// vector_utils.h: Utility classes implementing various vector operations

#include <cstddef>
#include <cmath>

namespace angle
{

template<size_t Dimension, typename Type>
class Vector;

using Vector2 = Vector<2, float>;
using Vector3 = Vector<3, float>;
using Vector4 = Vector<4, float>;

template<size_t Dimension, typename Type>
class VectorBase {
    public:
        using FullVector = Vector<Dimension, Type>;

        // Constructors
        VectorBase() = default;
        VectorBase(Type element);

        template<typename Type2>
        VectorBase(VectorBase<Dimension, Type2> other);

        template<typename Arg1, typename Arg2, typename ... Args>
        VectorBase(Arg1 arg1, Arg2 arg2, Args... args);

        // Access the vector backing storage directly
        const Type* data() const {return mData;}
        Type* data() {return mData;}
        constexpr size_t size() const {return Dimension;}

        // Load or store the pointer from / to raw data
        static FullVector Load(const Type* source);
        void store(Type* destination) const;

        // Index the vector
        Type& operator[](size_t i) {return mData[i];}
        const Type& operator[](size_t i) const {return mData[i];}

        // Basic arithmetic operations
        FullVector operator+() const;
        FullVector operator-() const;
        FullVector operator+(FullVector other) const;
        FullVector operator-(FullVector other) const;
        FullVector operator*(FullVector other) const;
        FullVector operator/(FullVector other) const;
        FullVector operator*(Type other) const;
        FullVector operator/(Type other) const;
        friend FullVector operator* (Type a, FullVector b) {return b * a;}

        // Compound arithmetic operations
        FullVector& operator+=(FullVector other);
        FullVector& operator-=(FullVector other);
        FullVector& operator*=(FullVector other);
        FullVector& operator/=(FullVector other);
        FullVector& operator*=(Type other);
        FullVector& operator/=(Type other);

        // Comparison operators
        bool operator==(FullVector other) const;
        bool operator!=(FullVector other) const;

        // Other arithmetic operations
        Type length() const;
        Type lengthSquared() const;
        Type dot(FullVector other) const;
        FullVector normalized() const;

    protected:
        //template<size_t L, typename T> friend class VectorBase;

        template<size_t CurrentIndex, size_t OtherDimension, typename OtherType, typename... Args>
        void InitWithList(Vector<OtherDimension, OtherType> arg1, Args... args);

        template<size_t CurrentIndex, typename... Args>
        void InitWithList(Type arg1, Args... args);

        template<size_t CurrentIndex>
        void InitWithList() const;

        template<size_t Dimension2, typename Type2>
        friend class VectorBase;

        Type mData[Dimension];
};

template<typename Type>
class Vector<2, Type>: public VectorBase<2, Type>
{
    public:
        using VectorBase<2, Type>::VectorBase;

        // Element shorthands
        Type& x() {return this->mData[0];}
        Type& y() {return this->mData[1];}

        const Type& x() const {return this->mData[0];}
        const Type& y() const {return this->mData[1];}
};

template<typename Type>
class Vector<3, Type>: public VectorBase<3, Type>
{
    public:
        using VectorBase<3, Type>::VectorBase;

        // Additional operations
        Vector<3, Type> cross(Vector<3, Type> other) const;

        // Element shorthands
        Type& x() {return this->mData[0];}
        Type& y() {return this->mData[1];}
        Type& z() {return this->mData[2];}

        const Type& x() const {return this->mData[0];}
        const Type& y() const {return this->mData[1];}
        const Type& z() const {return this->mData[2];}
};

template<typename Type>
class Vector<4, Type>: public VectorBase<4, Type>
{
    public:
        using VectorBase<4, Type>::VectorBase;

        // Element shorthands
        Type& x() {return this->mData[0];}
        Type& y() {return this->mData[1];}
        Type& z() {return this->mData[2];}
        Type& w() {return this->mData[3];}

        const Type& x() const {return this->mData[0];}
        const Type& y() const {return this->mData[1];}
        const Type& z() const {return this->mData[2];}
        const Type& w() const {return this->mData[3];}

        Type& r() {return this->mData[0];}
        Type& g() {return this->mData[1];}
        Type& b() {return this->mData[2];}
        Type& a() {return this->mData[3];}

        const Type& r() const {return this->mData[0];}
        const Type& g() const {return this->mData[1];}
        const Type& b() const {return this->mData[2];}
        const Type& a() const {return this->mData[3];}
};

// Implementation of constructors and misc operations

template<size_t Dimension, typename Type>
VectorBase<Dimension, Type>::VectorBase(Type element) {
    for (size_t i = 0; i < Dimension; ++i)
    {
        mData[i] = element;
    }
}

template<size_t Dimension, typename Type>
template<typename Type2>
VectorBase<Dimension, Type>::VectorBase(VectorBase<Dimension, Type2> other)
{
    for (size_t i = 0; i < Dimension; ++i)
    {
        mData[i] = other.mData[i];
    }
}

template<size_t Dimension, typename Type>
template<typename Arg1, typename Arg2, typename ... Args>
VectorBase<Dimension, Type>::VectorBase(Arg1 arg1, Arg2 arg2, Args... args)
{
    InitWithList<0>(arg1, arg2, args...);
}

template<size_t Dimension, typename Type>
template<size_t CurrentIndex, size_t OtherDimension, typename OtherType, typename... Args>
void VectorBase<Dimension, Type>::InitWithList(Vector<OtherDimension, OtherType> arg1, Args... args)
{
    static_assert(CurrentIndex + OtherDimension <= Dimension , "Too much data in the vector constructor.");
    for (size_t i = 0; i < OtherDimension; ++i)
    {
        mData[CurrentIndex + i] = arg1.mData[i];
    }
    InitWithList<CurrentIndex + OtherDimension>(args...);
}

template<size_t Dimension, typename Type>
template<size_t CurrentIndex, typename... Args>
void VectorBase<Dimension, Type>::InitWithList(Type arg1, Args... args)
{
    static_assert(CurrentIndex + 1 <= Dimension , "Too much data in the vector constructor.");
    mData[CurrentIndex] = arg1;
    InitWithList<CurrentIndex + 1>(args...);
}

template<size_t Dimension, typename Type>
template<size_t CurrentIndex>
void VectorBase<Dimension, Type>::InitWithList() const
{
    static_assert(CurrentIndex == Dimension, "Not enough data in the vector constructor.");
}

template<size_t Dimension, typename Type>
Vector<Dimension, Type> VectorBase<Dimension, Type>::Load(const Type* source)
{
    Vector<Dimension, Type> result;
    for (size_t i = 0; i < Dimension; ++i)
    {
        result.mData[i] = source[i];
    }
    return result;
}

template<size_t Dimension, typename Type>
void VectorBase<Dimension, Type>::store(Type* destination) const
{
    for (size_t i = 0; i < Dimension; ++i)
    {
        destination[i] = mData[i];
    }
}

// Implementation of basic arithmetic operations
template<size_t Dimension, typename Type>
Vector<Dimension, Type> VectorBase<Dimension, Type>::operator+() const
{
    Vector<Dimension, Type> result;
    for (size_t i = 0; i < Dimension; ++i)
    {
        result.mData[i] = +mData[i];
    }
    return result;
}

template<size_t Dimension, typename Type>
Vector<Dimension, Type> VectorBase<Dimension, Type>::operator-() const
{
    Vector<Dimension, Type> result;
    for (size_t i = 0; i < Dimension; ++i)
    {
        result.mData[i] = -mData[i];
    }
    return result;
}

template<size_t Dimension, typename Type>
Vector<Dimension, Type> VectorBase<Dimension, Type>::operator+(Vector<Dimension, Type> other) const
{
    Vector<Dimension, Type> result;
    for (size_t i = 0; i < Dimension; ++i)
    {
        result.mData[i] = mData[i] + other.mData[i];
    }
    return result;
}

template<size_t Dimension, typename Type>
Vector<Dimension, Type> VectorBase<Dimension, Type>::operator-(Vector<Dimension, Type> other) const
{
    Vector<Dimension, Type> result;
    for (size_t i = 0; i < Dimension; ++i)
    {
        result.mData[i] = mData[i] - other.mData[i];
    }
    return result;
}

template<size_t Dimension, typename Type>
Vector<Dimension, Type> VectorBase<Dimension, Type>::operator*(Vector<Dimension, Type> other) const
{
    Vector<Dimension, Type> result;
    for (size_t i = 0; i < Dimension; ++i)
    {
        result.mData[i] = mData[i] * other.mData[i];
    }
    return result;
}

template<size_t Dimension, typename Type>
Vector<Dimension, Type> VectorBase<Dimension, Type>::operator/(Vector<Dimension, Type> other) const
{
    Vector<Dimension, Type> result;
    for (size_t i = 0; i < Dimension; ++i)
    {
        result.mData[i] = mData[i] / other.mData[i];
    }
    return result;
}

template<size_t Dimension, typename Type>
Vector<Dimension, Type> VectorBase<Dimension, Type>::operator*(Type other) const
{
    Vector<Dimension, Type> result;
    for (size_t i = 0; i < Dimension; ++i)
    {
        result.mData[i] = mData[i] * other;
    }
    return result;
}

template<size_t Dimension, typename Type>
Vector<Dimension, Type> VectorBase<Dimension, Type>::operator/(Type other) const
{
    Vector<Dimension, Type> result;
    for (size_t i = 0; i < Dimension; ++i)
    {
        result.mData[i] = mData[i] / other;
    }
    return result;
}

template<size_t Dimension, typename Type>
Vector<Dimension, Type> operator*(Type a, Vector<Dimension, Type> b)
{
    Vector<Dimension, Type> result;
    for (size_t i = 0; i < Dimension; ++i)
    {
        result.mData[i] = a * b.mData[i];
    }
    return result;
}

template<size_t Dimension, typename Type>
Vector<Dimension, Type> operator/(Type a, Vector<Dimension, Type> b)
{
    Vector<Dimension, Type> result;
    for (size_t i = 0; i < Dimension; ++i)
    {
        result.mData[i] = a / b.mData[i];
    }
    return result;
}

// Implementation of compound arithmetic operations
template<size_t Dimension, typename Type>
Vector<Dimension, Type>& VectorBase<Dimension, Type>::operator+=(Vector<Dimension, Type> other)
{
    for (size_t i = 0; i < Dimension; ++i)
    {
        mData[i] += other.mData[i];
    }
    return *reinterpret_cast<Vector<Dimension, Type>*>(this);
}

template<size_t Dimension, typename Type>
Vector<Dimension, Type>& VectorBase<Dimension, Type>::operator-=(Vector<Dimension, Type> other)
{
    for (size_t i = 0; i < Dimension; ++i)
    {
        mData[i] -= other.mData[i];
    }
    return *reinterpret_cast<Vector<Dimension, Type>*>(this);
}

template<size_t Dimension, typename Type>
Vector<Dimension, Type>& VectorBase<Dimension, Type>::operator*=(Vector<Dimension, Type> other)
{
    for (size_t i = 0; i < Dimension; ++i)
    {
        mData[i] *= other.mData[i];
    }
    return *reinterpret_cast<Vector<Dimension, Type>*>(this);
}

template<size_t Dimension, typename Type>
Vector<Dimension, Type>& VectorBase<Dimension, Type>::operator/=(Vector<Dimension, Type> other)
{
    for (size_t i = 0; i < Dimension; ++i)
    {
        mData[i] /= other.mData[i];
    }
    return *reinterpret_cast<Vector<Dimension, Type>*>(this);
}

template<size_t Dimension, typename Type>
Vector<Dimension, Type>& VectorBase<Dimension, Type>::operator*=(Type other)
{
    for (size_t i = 0; i < Dimension; ++i)
    {
        mData[i] *= other;
    }
    return *reinterpret_cast<Vector<Dimension, Type>*>(this);
}

template<size_t Dimension, typename Type>
Vector<Dimension, Type>& VectorBase<Dimension, Type>::operator/=(Type other)
{
    for (size_t i = 0; i < Dimension; ++i)
    {
        mData[i] /= other;
    }
    return *reinterpret_cast<Vector<Dimension, Type>*>(this);
}

// Implementation of comparison operators
template<size_t Dimension, typename Type>
bool VectorBase<Dimension, Type>::operator==(Vector<Dimension, Type> other) const
{
    for (size_t i = 0; i < Dimension; ++i)
    {
        if (mData[i] != other.mData[i])
        {
            return false;
        }
    }
    return true;
}

template<size_t Dimension, typename Type>
bool VectorBase<Dimension, Type>::operator!=(Vector<Dimension, Type> other) const
{
    return !(*this == other);
}

// Implementation of other arithmetic operations
template<size_t Dimension, typename Type>
Type VectorBase<Dimension, Type>::length() const
{
    return std::sqrt(lengthSquared());
}

template<size_t Dimension, typename Type>
Type VectorBase<Dimension, Type>::lengthSquared() const
{
    return dot(*this);
}

template<size_t Dimension, typename Type>
Type VectorBase<Dimension, Type>::dot(Vector<Dimension, Type> other) const
{
    Type sum = Type();
    for (size_t i = 0; i < Dimension; ++i)
    {
        sum += mData[i] * other.mData[i];
    }
    return sum;
}

template<size_t Dimension, typename Type>
Vector<Dimension, Type> VectorBase<Dimension, Type>::normalized() const
{
    return *this / length();
}

template<typename Type>
Vector<3, Type> Vector<3, Type>::cross(Vector<3, Type> other) const
{
    return Vector<3, Type>(
        y() * other.z() - z() * other.y(),
        z() * other.x() - x() * other.z(),
        x() * other.y() - y() * other.x()
    );
}

} // namespace angle

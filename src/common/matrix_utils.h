//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Matrix:
//   Utility class implementing various matrix operations.
//   Supports matrices with minimum 2 and maximum 4 number of rows/columns.
//
// TODO: Check if we can merge Matrix.h in sample_util with this and replace it with this implementation.
// TODO: Rename this file to Matrix.h once we remove Matrix.h in sample_util.

#ifndef COMMON_MATRIX_UTILS_H_
#define COMMON_MATRIX_UTILS_H_

#include <vector>

#include "common/debug.h"
#include "common/mathutil.h"
#include "common/vector_utils.h"

namespace angle
{

template<typename T>
class Matrix
{
  public:
    Matrix(const std::vector<T> &elements, const unsigned int &numRows, const unsigned int &numCols)
        : mElements(elements),
          mRows(numRows),
          mCols(numCols)
    {
        ASSERT(rows() >= 1 && rows() <= 4);
        ASSERT(columns() >= 1 && columns() <= 4);
    }

    Matrix(const std::vector<T> &elements, const unsigned int &size)
        : mElements(elements),
          mRows(size),
          mCols(size)
    {
        ASSERT(rows() >= 1 && rows() <= 4);
        ASSERT(columns() >= 1 && columns() <= 4);
    }

    Matrix(const T *elements, const unsigned int &size)
        : mRows(size),
          mCols(size)
    {
        ASSERT(rows() >= 1 && rows() <= 4);
        ASSERT(columns() >= 1 && columns() <= 4);
        for (size_t i = 0; i < size * size; i++)
            mElements.push_back(elements[i]);
    }

    const T &operator()(const unsigned int &rowIndex, const unsigned int &columnIndex) const
    {
        return mElements[rowIndex * columns() + columnIndex];
    }

    T &operator()(const unsigned int &rowIndex, const unsigned int &columnIndex)
    {
        return mElements[rowIndex * columns() + columnIndex];
    }

    const T &at(const unsigned int &rowIndex, const unsigned int &columnIndex) const
    {
        return operator()(rowIndex, columnIndex);
    }

    Matrix<T> operator*(const Matrix<T> &m)
    {
        ASSERT(columns() == m.rows());

        unsigned int resultRows = rows();
        unsigned int resultCols = m.columns();
        Matrix<T> result(std::vector<T>(resultRows * resultCols), resultRows, resultCols);
        for (unsigned int i = 0; i < resultRows; i++)
        {
            for (unsigned int j = 0; j < resultCols; j++)
            {
                T tmp = 0.0f;
                for (unsigned int k = 0; k < columns(); k++)
                    tmp += at(i, k) * m(k, j);
                result(i, j) = tmp;
            }
        }

        return result;
    }

    unsigned int size() const
    {
        ASSERT(rows() == columns());
        return rows();
    }

    unsigned int rows() const { return mRows; }

    unsigned int columns() const { return mCols; }

    std::vector<T> elements() const { return mElements; }

    Matrix<T> compMult(const Matrix<T> &mat1) const
    {
        Matrix result(std::vector<T>(mElements.size()), size());
        for (unsigned int i = 0; i < columns(); i++)
            for (unsigned int j = 0; j < rows(); j++)
                result(i, j) = at(i, j) * mat1(i, j);

        return result;
    }

    Matrix<T> outerProduct(const Matrix<T> &mat1) const
    {
        unsigned int cols = mat1.columns();
        Matrix result(std::vector<T>(rows() * cols), rows(), cols);
        for (unsigned int i = 0; i < rows(); i++)
            for (unsigned int j = 0; j < cols; j++)
                result(i, j) = at(i, 0) * mat1(0, j);

        return result;
    }

    Matrix<T> transpose() const
    {
        Matrix result(std::vector<T>(mElements.size()), columns(), rows());
        for (unsigned int i = 0; i < columns(); i++)
            for (unsigned int j = 0; j < rows(); j++)
                result(i, j) = at(j, i);

        return result;
    }

    T determinant() const
    {
        ASSERT(rows() == columns());

        switch (size())
        {
          case 2:
            return at(0, 0) * at(1, 1) - at(0, 1) * at(1, 0);

          case 3:
            return at(0, 0) * at(1, 1) * at(2, 2) +
                at(0, 1) * at(1, 2) * at(2, 0) +
                at(0, 2) * at(1, 0) * at(2, 1) -
                at(0, 2) * at(1, 1) * at(2, 0) -
                at(0, 1) * at(1, 0) * at(2, 2) -
                at(0, 0) * at(1, 2) * at(2, 1);

          case 4:
            {
                const float minorMatrices[4][3 * 3] =
                {
                    {
                        at(1, 1), at(2, 1), at(3, 1),
                        at(1, 2), at(2, 2), at(3, 2),
                        at(1, 3), at(2, 3), at(3, 3),
                    },
                    {
                        at(1, 0), at(2, 0), at(3, 0),
                        at(1, 2), at(2, 2), at(3, 2),
                        at(1, 3), at(2, 3), at(3, 3),
                    },
                    {
                        at(1, 0), at(2, 0), at(3, 0),
                        at(1, 1), at(2, 1), at(3, 1),
                        at(1, 3), at(2, 3), at(3, 3),
                    },
                    {
                        at(1, 0), at(2, 0), at(3, 0),
                        at(1, 1), at(2, 1), at(3, 1),
                        at(1, 2), at(2, 2), at(3, 2),
                    }
              };
              return at(0, 0) * Matrix<T>(minorMatrices[0], 3).determinant() -
                  at(0, 1) * Matrix<T>(minorMatrices[1], 3).determinant() +
                  at(0, 2) * Matrix<T>(minorMatrices[2], 3).determinant() -
                  at(0, 3) * Matrix<T>(minorMatrices[3], 3).determinant();
            }

          default:
            UNREACHABLE();
            break;
        }

        return T();
    }

    Matrix<T> inverse() const
    {
        ASSERT(rows() == columns());

        Matrix<T> cof(std::vector<T>(mElements.size()), rows(), columns());
        switch (size())
        {
          case 2:
            cof(0, 0) = at(1, 1);
            cof(0, 1) = -at(1, 0);
            cof(1, 0) = -at(0, 1);
            cof(1, 1) = at(0, 0);
            break;

          case 3:
            cof(0, 0) = at(1, 1) * at(2, 2) -
                at(2, 1) * at(1, 2);
            cof(0, 1) = -(at(1, 0) * at(2, 2) -
                at(2, 0) * at(1, 2));
            cof(0, 2) = at(1, 0) * at(2, 1) -
                at(2, 0) * at(1, 1);
            cof(1, 0) = -(at(0, 1) * at(2, 2) -
                at(2, 1) * at(0, 2));
            cof(1, 1) = at(0, 0) * at(2, 2) -
                at(2, 0) * at(0, 2);
            cof(1, 2) = -(at(0, 0) * at(2, 1) -
                at(2, 0) * at(0, 1));
            cof(2, 0) = at(0, 1) * at(1, 2) -
                at(1, 1) * at(0, 2);
            cof(2, 1) = -(at(0, 0) * at(1, 2) -
                at(1, 0) * at(0, 2));
            cof(2, 2) = at(0, 0) * at(1, 1) -
                at(1, 0) * at(0, 1);
            break;

          case 4:
            cof(0, 0) = at(1, 1) * at(2, 2) * at(3, 3) +
                at(2, 1) * at(3, 2) * at(1, 3) +
                at(3, 1) * at(1, 2) * at(2, 3) -
                at(1, 1) * at(3, 2) * at(2, 3) -
                at(2, 1) * at(1, 2) * at(3, 3) -
                at(3, 1) * at(2, 2) * at(1, 3);
            cof(0, 1) = -(at(1, 0) * at(2, 2) * at(3, 3) +
                at(2, 0) * at(3, 2) * at(1, 3) +
                at(3, 0) * at(1, 2) * at(2, 3) -
                at(1, 0) * at(3, 2) * at(2, 3) -
                at(2, 0) * at(1, 2) * at(3, 3) -
                at(3, 0) * at(2, 2) * at(1, 3));
            cof(0, 2) = at(1, 0) * at(2, 1) * at(3, 3) +
                at(2, 0) * at(3, 1) * at(1, 3) +
                at(3, 0) * at(1, 1) * at(2, 3) -
                at(1, 0) * at(3, 1) * at(2, 3) -
                at(2, 0) * at(1, 1) * at(3, 3) -
                at(3, 0) * at(2, 1) * at(1, 3);
            cof(0, 3) = -(at(1, 0) * at(2, 1) * at(3, 2) +
                at(2, 0) * at(3, 1) * at(1, 2) +
                at(3, 0) * at(1, 1) * at(2, 2) -
                at(1, 0) * at(3, 1) * at(2, 2) -
                at(2, 0) * at(1, 1) * at(3, 2) -
                at(3, 0) * at(2, 1) * at(1, 2));
            cof(1, 0) = -(at(0, 1) * at(2, 2) * at(3, 3) +
                at(2, 1) * at(3, 2) * at(0, 3) +
                at(3, 1) * at(0, 2) * at(2, 3) -
                at(0, 1) * at(3, 2) * at(2, 3) -
                at(2, 1) * at(0, 2) * at(3, 3) -
                at(3, 1) * at(2, 2) * at(0, 3));
            cof(1, 1) = at(0, 0) * at(2, 2) * at(3, 3) +
                at(2, 0) * at(3, 2) * at(0, 3) +
                at(3, 0) * at(0, 2) * at(2, 3) -
                at(0, 0) * at(3, 2) * at(2, 3) -
                at(2, 0) * at(0, 2) * at(3, 3) -
                at(3, 0) * at(2, 2) * at(0, 3);
            cof(1, 2) = -(at(0, 0) * at(2, 1) * at(3, 3) +
                at(2, 0) * at(3, 1) * at(0, 3) +
                at(3, 0) * at(0, 1) * at(2, 3) -
                at(0, 0) * at(3, 1) * at(2, 3) -
                at(2, 0) * at(0, 1) * at(3, 3) -
                at(3, 0) * at(2, 1) * at(0, 3));
            cof(1, 3) = at(0, 0) * at(2, 1) * at(3, 2) +
                at(2, 0) * at(3, 1) * at(0, 2) +
                at(3, 0) * at(0, 1) * at(2, 2) -
                at(0, 0) * at(3, 1) * at(2, 2) -
                at(2, 0) * at(0, 1) * at(3, 2) -
                at(3, 0) * at(2, 1) * at(0, 2);
            cof(2, 0) = at(0, 1) * at(1, 2) * at(3, 3) +
                at(1, 1) * at(3, 2) * at(0, 3) +
                at(3, 1) * at(0, 2) * at(1, 3) -
                at(0, 1) * at(3, 2) * at(1, 3) -
                at(1, 1) * at(0, 2) * at(3, 3) -
                at(3, 1) * at(1, 2) * at(0, 3);
            cof(2, 1) = -(at(0, 0) * at(1, 2) * at(3, 3) +
                at(1, 0) * at(3, 2) * at(0, 3) +
                at(3, 0) * at(0, 2) * at(1, 3) -
                at(0, 0) * at(3, 2) * at(1, 3) -
                at(1, 0) * at(0, 2) * at(3, 3) -
                at(3, 0) * at(1, 2) * at(0, 3));
            cof(2, 2) = at(0, 0) * at(1, 1) * at(3, 3) +
                at(1, 0) * at(3, 1) * at(0, 3) +
                at(3, 0) * at(0, 1) * at(1, 3) -
                at(0, 0) * at(3, 1) * at(1, 3) -
                at(1, 0) * at(0, 1) * at(3, 3) -
                at(3, 0) * at(1, 1) * at(0, 3);
            cof(2, 3) = -(at(0, 0) * at(1, 1) * at(3, 2) +
                at(1, 0) * at(3, 1) * at(0, 2) +
                at(3, 0) * at(0, 1) * at(1, 2) -
                at(0, 0) * at(3, 1) * at(1, 2) -
                at(1, 0) * at(0, 1) * at(3, 2) -
                at(3, 0) * at(1, 1) * at(0, 2));
            cof(3, 0) = -(at(0, 1) * at(1, 2) * at(2, 3) +
                at(1, 1) * at(2, 2) * at(0, 3) +
                at(2, 1) * at(0, 2) * at(1, 3) -
                at(0, 1) * at(2, 2) * at(1, 3) -
                at(1, 1) * at(0, 2) * at(2, 3) -
                at(2, 1) * at(1, 2) * at(0, 3));
            cof(3, 1) = at(0, 0) * at(1, 2) * at(2, 3) +
                at(1, 0) * at(2, 2) * at(0, 3) +
                at(2, 0) * at(0, 2) * at(1, 3) -
                at(0, 0) * at(2, 2) * at(1, 3) -
                at(1, 0) * at(0, 2) * at(2, 3) -
                at(2, 0) * at(1, 2) * at(0, 3);
            cof(3, 2) = -(at(0, 0) * at(1, 1) * at(2, 3) +
                at(1, 0) * at(2, 1) * at(0, 3) +
                at(2, 0) * at(0, 1) * at(1, 3) -
                at(0, 0) * at(2, 1) * at(1, 3) -
                at(1, 0) * at(0, 1) * at(2, 3) -
                at(2, 0) * at(1, 1) * at(0, 3));
            cof(3, 3) = at(0, 0) * at(1, 1) * at(2, 2) +
                at(1, 0) * at(2, 1) * at(0, 2) +
                at(2, 0) * at(0, 1) * at(1, 2) -
                at(0, 0) * at(2, 1) * at(1, 2) -
                at(1, 0) * at(0, 1) * at(2, 2) -
                at(2, 0) * at(1, 1) * at(0, 2);
            break;

          default:
            UNREACHABLE();
            break;
        }

        // The inverse of A is the transpose of the cofactor matrix times the reciprocal of the determinant of A.
        Matrix<T> adjugateMatrix(cof.transpose());
        T det = determinant();
        Matrix<T> result(std::vector<T>(mElements.size()), rows(), columns());
        for (unsigned int i = 0; i < rows(); i++)
            for (unsigned int j = 0; j < columns(); j++)
                result(i, j) = det ? adjugateMatrix(i, j) / det : T();

        return result;
    }

    void setToIdentity()
    {
        ASSERT(rows() == columns());

        const auto one  = T(1);
        const auto zero = T(0);

        for (auto &e : mElements)
            e = zero;

        for (unsigned int i = 0; i < rows(); ++i)
        {
            const auto pos = i * columns() + (i % columns());
            mElements[pos] = one;
        }
    }

    template <unsigned int Size>
    static void setToIdentity(T(&matrix)[Size])
    {
        static_assert(gl::iSquareRoot<Size>() != 0, "Matrix is not square.");

        const auto cols = gl::iSquareRoot<Size>();
        const auto one  = T(1);
        const auto zero = T(0);

        for (auto &e : matrix)
            e = zero;

        for (unsigned int i = 0; i < cols; ++i)
        {
            const auto pos = i * cols + (i % cols);
            matrix[pos]    = one;
        }
    }

  protected:
    std::vector<T> mElements;
    unsigned int mRows;
    unsigned int mCols;
};

class Mat4 : public Matrix<float> {
public:
    Mat4() : Mat4(1.f, 0.f, 0.f, 0.f,
                  0.f, 1.f, 0.f, 0.f,
                  0.f, 0.f, 1.f, 0.f,
                  0.f, 0.f, 0.f, 1.f) { }
                  
    Mat4(float m00,
         float m01,
         float m02,
         float m03,
         float m10,
         float m11,
         float m12,
         float m13,
         float m20,
         float m21,
         float m22,
         float m23,
         float m30,
         float m31,
         float m32,
         float m33)
        : Matrix(std::vector<float>(16, 0), 4, 4) {
        mElements[0]  = m00;
        mElements[1]  = m01;
        mElements[2]  = m02;
        mElements[3]  = m03;
        mElements[4]  = m10;
        mElements[5]  = m11;
        mElements[6]  = m12;
        mElements[7]  = m13;
        mElements[8]  = m20;
        mElements[9]  = m21;
        mElements[10] = m22;
        mElements[11] = m23;
        mElements[12] = m30;
        mElements[13] = m31;
        mElements[14] = m32;
        mElements[15] = m33;
    }

    Mat4 identity() { return Mat4(); }

    Mat4 rotate(float angle, const Vector3 &axis) {
        float c = cos(angle);
        float ci = 1.f - c;
        float s = sin(angle);

        float x = axis.x();
        float y = axis.y();
        float z = axis.z();

        float x2 = x * x;
        float y2 = y * y;
        float z2 = z * z;

        float xy = x * y;
        float yz = y * z;
        float zx = z * x;

        float r00 = c + ci * x2;
        float r01 = ci * xy + s * z;
        float r02 = ci * zx - s * y;
        float r03 = 0.f;

        float r10 = ci * xy - s * z;
        float r11 = c + ci * y2;
        float r12 = ci * yz + s * x;
        float r13 = 0.f;

        float r20 = ci * zx + s * y;
        float r21 = ci * yz - s * x;
        float r22 = c + ci * z2;
        float r23 = 0.f;

        float r30 = 0.f;
        float r31 = 0.f;
        float r32 = 0.f;
        float r33 = 1.f;

        return Mat4(r00, r01, r02, r03,
                    r10, r11, r12, r13,
                    r20, r21, r22, r23,
                    r30, r31, r32, r33);
    }

    Mat4 translate(const Vector3 &t) {
        float r00 = 1.f;
        float r01 = 0.f;
        float r02 = 0.f;
        float r03 = 0.f;

        float r10 = 0.f;
        float r11 = 1.f;
        float r12 = 0.f;
        float r13 = 0.f;

        float r20 = 0.f;
        float r21 = 0.f;
        float r22 = 1.f;
        float r23 = 0.f;

        float r30 = t.x();
        float r31 = t.y();
        float r32 = t.z();
        float r33 = 1.f;

        return Mat4(r00, r01, r02, r03,
                    r10, r11, r12, r13,
                    r20, r21, r22, r23,
                    r30, r31, r32, r33);
    }

    Mat4 scale(const Vector3 &s) {
        float r00 = s.x();
        float r01 = 0.f;
        float r02 = 0.f;
        float r03 = 0.f;

        float r10 = 0.f;
        float r11 = s.y();
        float r12 = 0.f;
        float r13 = 0.f;

        float r20 = 0.f;
        float r21 = 0.f;
        float r22 = s.z();
        float r23 = 0.f;

        float r30 = 0.f;
        float r31 = 0.f;
        float r32 = 0.f;
        float r33 = 1.f;

        return Mat4(r00, r01, r02, r03,
                    r10, r11, r12, r13,
                    r20, r21, r22, r23,
                    r30, r31, r32, r33);
    }

    Mat4 frustum(float l, float r, float b, float t, float n, float f) {
        float r00 = s.x();
        float r01 = 0.f;
        float r02 = 0.f;
        float r03 = 0.f;

        float r10 = 0.f;
        float r11 = s.y();
        float r12 = 0.f;
        float r13 = 0.f;

        float r20 = 0.f;
        float r21 = 0.f;
        float r22 = s.z();
        float r23 = 0.f;

        float r30 = 0.f;
        float r31 = 0.f;
        float r32 = 0.f;
        float r33 = 1.f;

        return Mat4(r00, r01, r02, r03,
                    r10, r11, r12, r13,
                    r20, r21, r22, r23,
                    r30, r31, r32, r33);
    }

    Mat4 perspective(float fov, float aspectRatio, float n, float f);
    Mat4 ortho(float l, float r, float b, float t, float n, float f);
    Mat4 rollPitchYaw(float roll, float pitch, float yaw);

    Mat4 invert(const Mat4 &mat);
    Mat4 transpose(const Mat4 &mat);
    Vector3 transform(const Mat4 &mat, const Vector3 &pt);
    Vector3 transform(const Mat4 &mat, const Vector4 &pt);
};

} // namespace angle

#endif   // COMMON_MATRIX_UTILS_H_


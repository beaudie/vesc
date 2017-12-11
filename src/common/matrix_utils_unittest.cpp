//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// matrix_utils_unittests:
//   Unit tests for the matrix utils.
//

#include "matrix_utils.h"

#include <gtest/gtest.h>

using namespace angle;

namespace
{

const unsigned int minDimensions = 2;
const unsigned int maxDimensions = 4;

TEST(MatrixUtilsTest, MatrixConstructorTest)
{
    for (unsigned int i = minDimensions; i <= maxDimensions; i++)
    {
        for (unsigned int j = minDimensions; j <= maxDimensions; j++)
        {
            unsigned int numElements = i * j;
            Matrix<float> m(std::vector<float>(numElements, 1.0f), i, j);
            EXPECT_EQ(m.rows(), i);
            EXPECT_EQ(m.columns(), j);
            EXPECT_EQ(m.elements(), std::vector<float>(numElements, 1.0f));
        }
    }

    for (unsigned int i = minDimensions; i <= maxDimensions; i++)
    {
        unsigned int numElements = i * i;
        Matrix<float> m(std::vector<float>(numElements, 1.0f), i);
        EXPECT_EQ(m.size(), i);
        EXPECT_EQ(m.columns(), m.columns());
        EXPECT_EQ(m.elements(), std::vector<float>(numElements, 1.0f));
    }
}

TEST(MatrixUtilsTest, MatrixCompMultTest)
{
    for (unsigned int i = minDimensions; i <= maxDimensions; i++)
    {
        unsigned int numElements = i * i;
        Matrix<float> m1(std::vector<float>(numElements, 2.0f), i);
        Matrix<float> actualResult = m1.compMult(m1);
        std::vector<float> actualResultElements = actualResult.elements();
        std::vector<float> expectedResultElements(numElements, 4.0f);
        EXPECT_EQ(expectedResultElements, actualResultElements);
    }
}

TEST(MatrixUtilsTest, MatrixOuterProductTest)
{
    for (unsigned int i = minDimensions; i <= maxDimensions; i++)
    {
        for (unsigned int j = minDimensions; j <= maxDimensions; j++)
        {
            unsigned int numElements = i * j;
            Matrix<float> m1(std::vector<float>(numElements, 2.0f), i, 1);
            Matrix<float> m2(std::vector<float>(numElements, 2.0f), 1, j);
            Matrix<float> actualResult = m1.outerProduct(m2);
            EXPECT_EQ(actualResult.rows(), i);
            EXPECT_EQ(actualResult.columns(), j);
            std::vector<float> actualResultElements = actualResult.elements();
            std::vector<float> expectedResultElements(numElements, 4.0f);
            EXPECT_EQ(expectedResultElements, actualResultElements);
        }
    }
}

TEST(MatrixUtilsTest, MatrixTransposeTest)
{
    for (unsigned int i = minDimensions; i <= maxDimensions; i++)
    {
        for (unsigned int j = minDimensions; j <= maxDimensions; j++)
        {
            unsigned int numElements = i * j;
            Matrix<float> m1(std::vector<float>(numElements, 2.0f), i, j);
            Matrix<float> expectedResult = Matrix<float>(std::vector<float>(numElements, 2.0f), j, i);
            Matrix<float> actualResult = m1.transpose();
            EXPECT_EQ(expectedResult.elements(), actualResult.elements());
            EXPECT_EQ(actualResult.rows(), expectedResult.rows());
            EXPECT_EQ(actualResult.columns(), expectedResult.columns());
            // transpose(transpose(A)) = A
            Matrix<float> m2 = actualResult.transpose();
            EXPECT_EQ(m1.elements(), m2.elements());
        }
    }
}

TEST(MatrixUtilsTest, MatrixDeterminantTest)
{
    for (unsigned int i = minDimensions; i <= maxDimensions; i++)
    {
        unsigned int numElements = i * i;
        Matrix<float> m(std::vector<float>(numElements, 2.0f), i);
        EXPECT_EQ(m.determinant(), 0.0f);
    }
}

TEST(MatrixUtilsTest, 2x2MatrixInverseTest)
{
    float inputElements[] =
    {
        2.0f, 5.0f,
        3.0f, 7.0f
    };
    unsigned int numElements = 4;
    std::vector<float> input(inputElements, inputElements + numElements);
    Matrix<float> inputMatrix(input, 2);
    float identityElements[] =
    {
        1.0f, 0.0f,
        0.0f, 1.0f
    };
    std::vector<float> identityMatrix(identityElements, identityElements + numElements);
    // A * inverse(A) = I, where I is identity matrix.
    Matrix<float> result = inputMatrix * inputMatrix.inverse();
    EXPECT_EQ(identityMatrix, result.elements());
}

TEST(MatrixUtilsTest, 3x3MatrixInverseTest)
{
    float inputElements[] =
    {
        11.0f, 23.0f, 37.0f,
        13.0f, 29.0f, 41.0f,
        19.0f, 31.0f, 43.0f
    };
    unsigned int numElements = 9;
    std::vector<float> input(inputElements, inputElements + numElements);
    Matrix<float> inputMatrix(input, 3);
    float identityElements[] =
    {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };
    std::vector<float> identityMatrix(identityElements, identityElements + numElements);
    // A * inverse(A) = I, where I is identity matrix.
    Matrix<float> result = inputMatrix * inputMatrix.inverse();
    std::vector<float> resultElements = result.elements();
    const float floatFaultTolarance = 0.000001f;
    for (size_t i = 0; i < numElements; i++)
        EXPECT_NEAR(resultElements[i], identityMatrix[i], floatFaultTolarance);
}

TEST(MatrixUtilsTest, 4x4MatrixInverseTest)
{
    float inputElements[] =
    {
        29.0f, 43.0f, 61.0f, 79.0f,
        31.0f, 47.0f, 67.0f, 83.0f,
        37.0f, 53.0f, 71.0f, 89.0f,
        41.0f, 59.0f, 73.0f, 97.0f
    };
    unsigned int numElements = 16;
    std::vector<float> input(inputElements, inputElements + numElements);
    Matrix<float> inputMatrix(input, 4);
    float identityElements[] =
    {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };
    std::vector<float> identityMatrix(identityElements, identityElements + numElements);
    // A * inverse(A) = I, where I is identity matrix.
    Matrix<float> result = inputMatrix * inputMatrix.inverse();
    std::vector<float> resultElements = result.elements();
    const float floatFaultTolarance = 0.00001f;
    for (unsigned int i = 0; i < numElements; i++)
        EXPECT_NEAR(resultElements[i], identityMatrix[i], floatFaultTolarance);
}

static void checkMat4ExactlyEq(const Matrix<float>& a, const Matrix<float>& b) {
    for (unsigned int i = 0; i < 4; i++) {
        for (unsigned int j = 0; i < 4; i++) {
            EXPECT_EQ(a.at(i, j), b.at(i, j));
        }
    }
}

TEST(MatrixUtilsTest, Mat4Construction) {
    float elements[] =
    {
        0.0f, 1.0f, 2.0f, 3.0f,
        4.0f, 5.0f, 6.0f, 7.0f,
        8.0f, 9.0f, 10.0f, 11.0f,
        12.0f, 13.0f, 14.0f, 15.0f,
    };

    std::vector<float> elementsVector(16, 0);
    for (int i = 0; i < 16; i++) {
        elementsVector[i] = elements[i];
    }

    Matrix<float> a(elements, 4);
    Mat4 b(elements);
    Mat4 bVec(elementsVector);

    checkMat4ExactlyEq(a, b);
    checkMat4ExactlyEq(b, bVec);

    a.setToIdentity();
    b = Mat4();

    checkMat4ExactlyEq(a, b);

    Mat4 c(0.0f, 1.0f, 2.0f, 3.0f,
           4.0f, 5.0f, 6.0f, 7.0f,
           8.0f, 9.0f, 10.0f, 11.0f,
           12.0f, 13.0f, 14.0f, 15.0f);
    Mat4 d(elements);
    Mat4 e(Matrix<float>(elements, 4));

    checkMat4ExactlyEq(c, d);
    checkMat4ExactlyEq(e, d);
}

struct RotateArgs {
    float angle;
    Vector3 axis;
};

// TODO: Spec out requirements for matrix precision
void checkMatrixCloseToGolden(float* golden, const Mat4& m) {
    const float floatFaultTolarance = 0.000001f;
    const auto& checkElts = m.elements();
    for (size_t i = 0; i < checkElts.size(); i++) {
        EXPECT_NEAR(golden[i], checkElts[i], floatFaultTolarance);
    }
}

void checkMatrixCloseToGolden(const std::vector<float>& golden, const Mat4& m) {
    const float floatFaultTolarance = 0.000001f;
    const auto& checkElts = m.elements();
    for (size_t i = 0; i < golden.size(); i++) {
        EXPECT_NEAR(golden[i], checkElts[i], floatFaultTolarance);
    }
}

TEST(MatrixUtilsTest, Mat4Rotate) {
    // Sanity check.
    float elementsExpected[] =
    {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };

    std::vector<float> elementsExpectedVector(16, 0);
    for (int i = 0; i < 16; i++) {
        elementsExpectedVector[i] = elementsExpected[i];
    }

    Mat4 r = Mat4::rotate(0.f, Vector3(0.f, 0.f, 1.f));
    Mat4 golden(elementsExpected);
    checkMat4ExactlyEq(r, golden);
    checkMatrixCloseToGolden(elementsExpected, r);
    checkMatrixCloseToGolden(elementsExpectedVector, r);

    // Randomly-generated inputs, outputs using GLM.
    std::vector<RotateArgs> rotationGoldenInputs;
    std::vector<std::vector<float> > rotationGoldenOutputs;
    rotationGoldenInputs.push_back({ -123.202621,  Vector3(1.49511719, 8.43707085, 1.84892273)});
    rotationGoldenInputs.push_back({ -185.304932,  Vector3(8.33139801, 5.83176517, -5.52017164)});
    rotationGoldenInputs.push_back({ 89.1301575,  Vector3(-8.59627247, 2.45473671, -3.24616003)});
    rotationGoldenInputs.push_back({ 64.2547302,  Vector3(-1.96404457, -9.6942234, 9.19210434)});
    rotationGoldenInputs.push_back({ -298.558502,  Vector3(-5.36005449, -6.43335342, -5.37347507)});
    rotationGoldenInputs.push_back({ 288.26062,  Vector3(-2.30438757, -9.84476185, -0.912479401)});
    rotationGoldenInputs.push_back({ 142.39563,  Vector3(-1.00449753, -2.583498, -0.845117569)});
    rotationGoldenInputs.push_back({ -140.044586,  Vector3(-1.37100601, -2.50429916, -9.75720882)});
    rotationGoldenInputs.push_back({ -338.544342,  Vector3(6.80566216, 2.75088787, -5.83434296)});
    rotationGoldenInputs.push_back({ 79.0578613,  Vector3(9.05184937, -5.56159019, 6.35597992)});
    rotationGoldenInputs.push_back({ 4464.63672,  Vector3(-53.9424286, -10.361496, 54.3564453)});
    rotationGoldenInputs.push_back({ -2820.63477,  Vector3(62.1694794, 82.497757, -60.0084801)});
    rotationGoldenInputs.push_back({ 3371.05273,  Vector3(-74.5660324, -31.3026276, 96.725235)});
    rotationGoldenInputs.push_back({ 5501.7168,  Vector3(15.030838, 23.2323914, 66.8295593)});
    rotationGoldenInputs.push_back({ 392.175781,  Vector3(36.5722198, 69.2820892, 24.1789474)});
    rotationGoldenInputs.push_back({ -2206.71387,  Vector3(-91.5292282, 68.2716675, 42.0627289)});
    rotationGoldenInputs.push_back({ -4648.8623,  Vector3(50.7790375, 43.3964081, -36.3877525)});
    rotationGoldenInputs.push_back({ 2794.60156,  Vector3(76.2934265, 63.4901886, 79.5993042)});
    rotationGoldenInputs.push_back({ 2294.67871,  Vector3(-81.3662338, 77.694458, 10.8423462)});
    rotationGoldenInputs.push_back({ 2451.04688,  Vector3(-80.6299896, 51.8244629, 13.6877518)});
    rotationGoldenOutputs.push_back({-0.502578616, 0.077577576, 0.861043811, 0, 0.430558145, 0.886124551, 0.17147316, 0, -0.749689579, 0.456908166, -0.478748918, 0, 0, 0, 0, 1, });
    rotationGoldenOutputs.push_back({0.0388860106, 0.68008846, -0.732098162, 0, 0.768302619, -0.48879838, -0.413264215, 0, -0.638904691, -0.546402752, -0.541520953, -0, 0, 0, 0, 1, });
    rotationGoldenOutputs.push_back({0.819677353, -0.570997, 0.0457325876, 0, 0.111535847, 0.0807825029, -0.990471601, 0, 0.561861873, 0.816967964, 0.129902199, 0, 0, 0, 0, 1, });
    rotationGoldenOutputs.push_back({0.44633761, 0.672237933, 0.590659797, 0, -0.554105937, 0.725911558, -0.407454431, 0, -0.702673018, -0.145425856, 0.696492612, 0, 0, 0, 0, 1, });
    rotationGoldenOutputs.push_back({0.629537523, -0.292549372, 0.719789803, 0, 0.656178474, 0.696276903, -0.29090941, 0, -0.41606766, 0.655448973, 0.630297065, 0, 0, 0, 0, 1, });
    rotationGoldenOutputs.push_back({0.348719478, 0.236504108, -0.906896055, 0, 0.0657925531, 0.959072948, 0.27540952, 0, 0.934915006, -0.155707672, 0.318887234, 0, 0, 0, 0, 1, });
    rotationGoldenOutputs.push_back({-0.576898336, 0.375895619, 0.725183249, 0, 0.731807947, 0.632225156, 0.254457355, 0, -0.362829685, 0.67749095, -0.639813006, 0, 0, 0, 0, 1, });
    rotationGoldenOutputs.push_back({-0.734417081, 0.6750319, 0.0704519302, 0, -0.557663441, -0.659350932, 0.504249811, 0, 0.386837244, 0.331041187, 0.860678971, 0, 0, 0, 0, 1, });
    rotationGoldenOutputs.push_back({0.967206657, -0.212837532, -0.138605639, 0, 0.242349193, 0.936665297, 0.252833962, 0, 0.0760145336, -0.278133631, 0.957529902, 0, 0, 0, 0, 1, });
    rotationGoldenOutputs.push_back({0.622941673, 0.237952977, 0.745199323, 0, -0.770188689, 0.353324354, 0.531009674, 0, -0.136941731, -0.904732108, 0.403369129, 0, 0, 0, 0, 1, });
    rotationGoldenOutputs.push_back({0.069116354, 0.57701838, -0.813801348, 0, -0.23717764, -0.782857955, -0.575221837, -0, -0.969004452, 0.232772723, 0.0827475786, 0, 0, 0, 0, 1, });
    rotationGoldenOutputs.push_back({0.642379999, -0.255957425, -0.722380519, 0, 0.608449876, 0.743438125, 0.277648032, 0, 0.465979189, -0.617887914, 0.63330704, 0, 0, 0, 0, 1, });
    rotationGoldenOutputs.push_back({-0.0772442222, 0.821814537, -0.56449455, 0, -0.335262537, -0.55462563, -0.761570334, -0, -0.938952804, 0.130427033, 0.318365276, 0, 0, 0, 0, 1, });
    rotationGoldenOutputs.push_back({-0.15112105, 0.98498112, -0.0835132897, 0, -0.824382067, -0.0789582208, 0.560499489, 0, 0.545487404, 0.153550103, 0.823933125, 0, 0, 0, 0, 1, });
    rotationGoldenOutputs.push_back({0.876976967, 0.214932591, -0.429785222, 0, -0.0991527736, 0.95608449, 0.275809884, 0, 0.470191568, -0.199264526, 0.859775245, 0, 0, 0, 0, 1, });
    rotationGoldenOutputs.push_back({0.863490582, -0.384278059, 0.32667169, 0, 0.118962884, 0.78459096, 0.608493924, 0, -0.490134507, -0.486566991, 0.723201752, 0, 0, 0, 0, 1, });
    rotationGoldenOutputs.push_back({0.920134842, -0.192495584, -0.341023833, 0, 0.302240282, 0.902822196, 0.305880517, 0, 0.249003246, -0.384522468, 0.888898134, 0, 0, 0, 0, 1, });
    rotationGoldenOutputs.push_back({0.41095221, -0.348385721, 0.842464209, 0, 0.898837209, 0.309263796, -0.310560435, 0, -0.152348787, 0.884863555, 0.440234661, 0, 0, 0, 0, 1, });
    rotationGoldenOutputs.push_back({0.179531276, -0.77461791, -0.606411994, 0, -0.911041379, 0.101665795, -0.399584115, 0, 0.371176422, 0.624204338, -0.687456906, 0, 0, 0, 0, 1, });
    rotationGoldenOutputs.push_back({0.803580999, -0.41760689, 0.424101382, 0, -0.153726518, 0.542743206, 0.825710595, 0, -0.575000584, -0.728720963, 0.371940881, 0, 0, 0, 0, 1, });

    EXPECT_EQ(rotationGoldenInputs.size(), rotationGoldenOutputs.size());

    for (size_t i = 0; i < rotationGoldenInputs.size(); i++) {
        const auto& input = rotationGoldenInputs[i];
        const auto& output = rotationGoldenOutputs[i];
        Mat4 r = Mat4::rotate(input.angle, input.axis);
        checkMatrixCloseToGolden(output, r);
    }

}

TEST(MatrixUtilsTest, Mat4Translate) {

}

}


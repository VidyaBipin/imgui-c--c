#include <immat.h>
#include <iostream>

int main(int argc, char ** argv)
{
    int mw = 4;
    int mh = 4;
    ImGui::ImMat A, B, C, X;
    A.create_type(mw, mh, IM_DT_FLOAT32);
    B.create_type(mw, mh, IM_DT_FLOAT32);
    X.create_type(mw, mh, IM_DT_INT8);

    for (int y = 0; y < A.h; y++)
    {
        for (int x = 0; x < A.w; x++)
        {
            A.at<float>(x,y) += y * A.w + x + 1;
        }
    }

    for (int y = 0; y < B.h; y++)
    {
        for (int x = 0; x < B.w; x++)
        {
            B.at<float>(x,y) = y * B.w + x + 1;
        }
    }

    A.print("A");
    B.print("B");

    // scalar math
    C = B + 2.f;
    C.print("C=B+2");

    B += 2.f;
    B.print("B+=2");

    C = B - 2.f;
    C.print("C=B-2");

    B -= 2.f;
    B.print("B-=2");

    C = A * 2.0f;
    C.print("C=A*2");

    A *= 2.0f;
    A.print("A*=2");

    C = A / 2.0f;
    C.print("C=A/2");

    A /= 2.0f;
    A.print("A/=2");

    // mat math
    C = A + B;
    C.print("C=A+B");

    A += B;
    A.print("A+=B");

    C = A - B;
    C.print("C=A-B");

    A -= B;
    A.print("A-=B");

    C = A * B;
    C.print("C=A*B");

    A *= B;
    A.print("A*=B");

    C = A.clip(200, 500);
    C.print("C=A.clip(200,500)");

    // mat tranform
    auto t = A.t();
    t.print("A.t");

    // mat setting
    auto e = A.eye(1.f);
    e.print("A.eye");

    auto n = A.randn<float>(0.f, 5.f);
    n.print("A.randn");

    // mat matrix math
    C = n.inv<float>();
    C.print("C=A.randn.i");

    X = X.randn(128.f, 128.f);
    X.print("INT8 randn");

    // fp16
    ImGui::ImMat A16, B16, C16;
    A16.create_type(mw, mh, IM_DT_FLOAT16);
    B16.create_type(mw, mh, IM_DT_FLOAT16);
    for (int y = 0; y < A16.h; y++)
    {
        for (int x = 0; x < A16.w; x++)
        {
            A16.at<uint16_t>(x,y) = im_float32_to_float16(y * A16.w + x + 1);
        }
    }
    for (int y = 0; y < B16.h; y++)
    {
        for (int x = 0; x < B16.w; x++)
        {
            B16.at<uint16_t>(x,y) = im_float32_to_float16(y * B16.w + x + 1);
        }
    }

    A16.print("A16");
    B16.print("B16");

    // fp16 scalar math
    C16 = B16 + 2.f;
    C16.print("C16=B16+2");

    B16 += 2.f;
    B16.print("B16+=2");

    C16 = B16 - 2.f;
    C16.print("C16=B16-2");

    B16 -= 2.f;
    B16.print("B16-=2");

    C16 = A16 * 2.0f;
    C16.print("C16=A16*2");

    A16 *= 2.0f;
    A16.print("A16*=2");

    C16 = A16 / 2.0f;
    C16.print("C16=A16/2");

    A16 /= 2.0f;
    A16.print("A16/=2");

    // mat math
    C16 = A16 + B16;
    C16.print("C16=A16+B16");

    A16 += B16;
    A16.print("A16+=B16");

    C16 = A16 - B16;
    C16.print("C16=A16-B16");

    A16 -= B16;
    A16.print("A16-=B16");

    C16 = A16 * B16;
    C16.print("C16=A16*B16");

    A16 *= B16;
    A16.print("A16*=B16");

    C16 = A16.clip(200, 500);
    C16.print("C16=A16.clip(200,500)");

    // mat tranform
    auto t16 = A16.t();
    t16.print("A16.t");

    // mat setting
    auto e16 = A16.eye(1.f);
    e16.print("A16.eye");

    auto n16 = A16.randn<float>(0.f, 5.f);
    n16.print("A16.randn");

    // mat matrix math
    //C16 = n16.inv<float>();
    //C16.print("C16=A16.randn.i");

    A.create_type(2, 2, IM_DT_FLOAT32);
    A.at<float>(0, 0) = 3.f;
    A.at<float>(1, 0) = 8.f;
    A.at<float>(0, 1) = 4.f;
    A.at<float>(1, 1) = 6.f;
    A.print("A");
    auto ra = A.determinant();
    fprintf(stderr, "A.determinant:%f\n", ra);

    A.at<float>(0, 0) = 4.f;
    A.at<float>(1, 0) = 6.f;
    A.at<float>(0, 1) = 3.f;
    A.at<float>(1, 1) = 8.f;
    A.print("A");
    auto ra2 = A.determinant();
    fprintf(stderr, "A.determinant:%f\n", ra2);

    B.create_type(3, 3, IM_DT_FLOAT32);
    B.at<float>(0, 0) = 6.f;
    B.at<float>(1, 0) = 1.f;
    B.at<float>(2, 0) = 1.f;
    B.at<float>(0, 1) = 4.f;
    B.at<float>(1, 1) = -2.f;
    B.at<float>(2, 1) = 5.f;
    B.at<float>(0, 2) = 2.f;
    B.at<float>(1, 2) = 8.f;
    B.at<float>(2, 2) = 7.f;
    B.print("B");
    auto rb = B.determinant();
    fprintf(stderr, "B.determinant:%f\n", rb);

    C.create_type(2, 6, IM_DT_FLOAT32);
    C.at<float>(0, 0) = 0.f;
    C.at<float>(0, 1) = 1.f;
    C.at<float>(0, 2) = 2.f;
    C.at<float>(0, 3) = 3.f;
    C.at<float>(0, 4) = 4.f;
    C.at<float>(0, 5) = 5.f;
    C.at<float>(1, 0) = 6.f;
    C.at<float>(1, 1) = 7.f;
    C.at<float>(1, 2) = 8.f;
    C.at<float>(1, 3) = 9.f;
    C.at<float>(1, 4) = 10.f;
    C.at<float>(1, 5) = 11.f;
    C.print("C");

    ImGui::ImMat U, S, V;
    ImGui::SVD(C, S, U, V);
    S.print("S");
    U.print("U");
    V.print("V");

    return 0;
}
#include <immat.h>
#include <iostream>

int main(int argc, char ** argv)
{
    int mw = 4;
    int mh = 4;
    ImGui::ImMat A, B, C;
    A.create_type(mw, mh, IM_DT_FLOAT32);
    B.create_type(mw, mh, IM_DT_FLOAT32);

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

    return 0;
}
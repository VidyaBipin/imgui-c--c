#include "ImGuiZmo.h"
#include <imgui_extra_widget.h>
#include <string>
#include <vector>
#include <algorithm>

namespace IMGUIZMO_NAMESPACE
{
static int gizmoCount = 1;
static float camDistance = 8.f;
static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);

static float objectMatrix[4][16] =
{
    {   1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f
    },

    {   1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        2.f, 0.f, 0.f, 1.f
    },

    { 1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        2.f, 0.f, 2.f, 1.f
    },

    {   1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 2.f, 1.f 
    }
};

static const float identityMatrix[16] =
{   1.f, 0.f, 0.f, 0.f,
    0.f, 1.f, 0.f, 0.f,
    0.f, 0.f, 1.f, 0.f,
    0.f, 0.f, 0.f, 1.f
};

static void Frustum(float left, float right, float bottom, float top, float znear, float zfar, float* m16)
{
    float temp, temp2, temp3, temp4;
    temp = 2.0f * znear;
    temp2 = right - left;
    temp3 = top - bottom;
    temp4 = zfar - znear;
    m16[0] = temp / temp2;
    m16[1] = 0.0;
    m16[2] = 0.0;
    m16[3] = 0.0;
    m16[4] = 0.0;
    m16[5] = temp / temp3;
    m16[6] = 0.0;
    m16[7] = 0.0;
    m16[8] = (right + left) / temp2;
    m16[9] = (top + bottom) / temp3;
    m16[10] = (-zfar - znear) / temp4;
    m16[11] = -1.0f;
    m16[12] = 0.0;
    m16[13] = 0.0;
    m16[14] = (-temp * zfar) / temp4;
    m16[15] = 0.0;
}

static void Perspective(float fovyInDegrees, float aspectRatio, float znear, float zfar, float* m16)
{
    float ymax, xmax;
    ymax = znear * tanf(fovyInDegrees * 3.141592f / 180.0f);
    xmax = ymax * aspectRatio;
    Frustum(-xmax, xmax, -ymax, ymax, znear, zfar, m16);
}

static void Cross(const float* a, const float* b, float* r)
{
    r[0] = a[1] * b[2] - a[2] * b[1];
    r[1] = a[2] * b[0] - a[0] * b[2];
    r[2] = a[0] * b[1] - a[1] * b[0];
}

static float Dot(const float* a, const float* b)
{
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

static void Normalize(const float* a, float* r)
{
    float il = 1.f / (sqrtf(Dot(a, a)) + FLT_EPSILON);
    r[0] = a[0] * il;
    r[1] = a[1] * il;
    r[2] = a[2] * il;
}

static void LookAt(const float* eye, const float* at, const float* up, float* m16)
{
    float X[3], Y[3], Z[3], tmp[3]; 
    tmp[0] = eye[0] - at[0];
    tmp[1] = eye[1] - at[1];
    tmp[2] = eye[2] - at[2];
    Normalize(tmp, Z);
    Normalize(up, Y);   
    Cross(Y, Z, tmp);
    Normalize(tmp, X);  
    Cross(Z, X, tmp);
    Normalize(tmp, Y);  
    m16[0] = X[0];
    m16[1] = Y[0];
    m16[2] = Z[0];
    m16[3] = 0.0f;
    m16[4] = X[1];
    m16[5] = Y[1];
    m16[6] = Z[1];
    m16[7] = 0.0f;
    m16[8] = X[2];
    m16[9] = Y[2];
    m16[10] = Z[2];
    m16[11] = 0.0f;
    m16[12] = -Dot(X, eye);
    m16[13] = -Dot(Y, eye);
    m16[14] = -Dot(Z, eye);
    m16[15] = 1.0f;
}

static void OrthoGraphic(const float l, float r, float b, const float t, float zn, const float zf, float* m16)
{
    m16[0] = 2 / (r - l);
    m16[1] = 0.0f;
    m16[2] = 0.0f;
    m16[3] = 0.0f;
    m16[4] = 0.0f;
    m16[5] = 2 / (t - b);
    m16[6] = 0.0f;
    m16[7] = 0.0f;
    m16[8] = 0.0f;
    m16[9] = 0.0f;
    m16[10] = 1.0f / (zf - zn);
    m16[11] = 0.0f;
    m16[12] = (l + r) / (l - r);
    m16[13] = (t + b) / (b - t);
    m16[14] = zn / (zn - zf);
    m16[15] = 1.0f;
}

inline void rotationY(const float angle, float* m16)
{
    float c = cosf(angle);
    float s = sinf(angle);  
    m16[0] = c;
    m16[1] = 0.0f;
    m16[2] = -s;
    m16[3] = 0.0f;
    m16[4] = 0.0f;
    m16[5] = 1.f;
    m16[6] = 0.0f;
    m16[7] = 0.0f;
    m16[8] = s;
    m16[9] = 0.0f;
    m16[10] = c;
    m16[11] = 0.0f;
    m16[12] = 0.f;
    m16[13] = 0.f;
    m16[14] = 0.f;
    m16[15] = 1.0f;
}

static void EditTransform(float* cameraView, float* cameraProjection, float* matrix)
{
    static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::LOCAL);
    static bool useSnap = false;
    static float snap[3] = { 1.f, 1.f, 1.f };
    static float bounds[] = { -0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f };
    static float boundsSnap[] = { 0.1f, 0.1f, 0.1f };
    static bool boundSizing = false;
    static bool boundSizingSnap = false;

    ImVec2 window_size = ImGui::GetWindowSize();
    static float size_ctrl = 0.25 * window_size.x, size_zmo = 0.75 * window_size.x;
    ImGui::PushID("##ImGuizmoCtrl");
    ImGui::Splitter(true, 8.0f, &size_ctrl, &size_zmo, 8, 8);
    ImGui::PopID();
    ImGui::BeginChild("Control", ImVec2(size_ctrl, window_size.y));
    {
        if (ImGui::IsKeyPressed(ImGuiKey_T))
            mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
        if (ImGui::IsKeyPressed(ImGuiKey_E))
            mCurrentGizmoOperation = ImGuizmo::ROTATE;
        if (ImGui::IsKeyPressed(ImGuiKey_R)) // r Key
            mCurrentGizmoOperation = ImGuizmo::SCALE;
        if (ImGui::RadioButton("Translate", mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
            mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
        ImGui::SameLine();
        if (ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
            mCurrentGizmoOperation = ImGuizmo::ROTATE;
        ImGui::SameLine();
        if (ImGui::RadioButton("Scale", mCurrentGizmoOperation == ImGuizmo::SCALE))
            mCurrentGizmoOperation = ImGuizmo::SCALE;
        if (ImGui::RadioButton("Universal", mCurrentGizmoOperation == ImGuizmo::UNIVERSAL))
            mCurrentGizmoOperation = ImGuizmo::UNIVERSAL;
        float matrixTranslation[3], matrixRotation[3], matrixScale[3];
        ImGuizmo::DecomposeMatrixToComponents(matrix, matrixTranslation, matrixRotation, matrixScale);
        ImGui::InputFloat3("Tr", matrixTranslation);
        ImGui::InputFloat3("Rt", matrixRotation);
        ImGui::InputFloat3("Sc", matrixScale);
        ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, matrix);

        if (mCurrentGizmoOperation != ImGuizmo::SCALE)
        {
            if (ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
                mCurrentGizmoMode = ImGuizmo::LOCAL;
            ImGui::SameLine();
            if (ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
                mCurrentGizmoMode = ImGuizmo::WORLD;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_S))
            useSnap = !useSnap;
        ImGui::Checkbox("##snap", &useSnap);
        ImGui::SameLine();

        switch (mCurrentGizmoOperation)
        {
        case ImGuizmo::TRANSLATE:
            ImGui::InputFloat3("Snap", &snap[0]);
            break;
        case ImGuizmo::ROTATE:
            ImGui::InputFloat("Angle Snap", &snap[0]);
            break;
        case ImGuizmo::SCALE:
            ImGui::InputFloat("Scale Snap", &snap[0]);
            break;
        default:
            break;
        }
        ImGui::Checkbox("Bound Sizing", &boundSizing);
        if (boundSizing)
        {
            ImGui::PushID(3);
            ImGui::Checkbox("##BoundSizing", &boundSizingSnap);
            ImGui::SameLine();
            ImGui::InputFloat3("Snap", boundsSnap);
            ImGui::PopID();
        }
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("zmo", ImVec2(size_zmo, window_size.y), false, ImGuiWindowFlags_NoMove);
    {
        ImGui::PushStyleColor(ImGuiCol_WindowBg, (ImVec4)ImColor(0.35f, 0.3f, 0.3f));
        ImGuizmo::SetDrawlist();
        float windowWidth = (float)ImGui::GetWindowWidth();
        float windowHeight = (float)ImGui::GetWindowHeight();
        ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);
        float viewManipulateRight = ImGui::GetWindowPos().x + windowWidth;
        float viewManipulateTop = ImGui::GetWindowPos().y;
        ImGuizmo::DrawGrid(cameraView, cameraProjection, identityMatrix, 100.f);
        ImGuizmo::DrawCubes(cameraView, cameraProjection, &objectMatrix[0][0], gizmoCount);
        ImGuizmo::Manipulate(cameraView, cameraProjection, mCurrentGizmoOperation, mCurrentGizmoMode, matrix, NULL, useSnap ? &snap[0] : NULL, boundSizing ? bounds : NULL, boundSizingSnap ? boundsSnap : NULL);
        ImGuizmo::ViewManipulate(cameraView, camDistance, ImVec2(viewManipulateRight - 128, viewManipulateTop), ImVec2(128, 128), 0x10101010);
        ImGui::PopStyleColor(1);
    }
    ImGui::EndChild();
}

void ShowImGuiZmoDemo()
{
    static float size_edit = 0;
    static float size_edit_h = 0; 
    static float size_zmo_h = 0;
    static float cameraView[16] =
    {   1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f
    };
    static float cameraProjection[16];
    // Camera projection
    static bool isPerspective = true;
    static float fov = 27.f;
    static float viewWidth = 10.f; // for orthographic
    static float camYAngle = 165.f / 180.f * 3.14159f;
    static float camXAngle = 32.f / 180.f * 3.14159f;

    static bool firstFrame = true;
    ImVec2 window_size = ImGui::GetWindowSize();
    if (isPerspective)
    {
        Perspective(fov, window_size.x / window_size.y, 0.1f, 100.f, cameraProjection);
    }
    else
    {
        float viewHeight = viewWidth * window_size.y / window_size.x;
        OrthoGraphic(-viewWidth, viewWidth, -viewHeight, viewHeight, 1000.f, -1000.f, cameraProjection);
    }
    ImGuizmo::SetOrthographic(!isPerspective);

    if (size_edit == 0 || size_edit_h == 0 || size_zmo_h == 0)
    {
        size_edit = window_size.y;
        size_edit_h = 0.25 * size_edit;
        size_zmo_h = size_edit - size_edit_h;
    }

    ImGui::PushID("##ImGuizmoView");
    ImGui::Splitter(false, 8.0f, &size_edit_h, &size_zmo_h, 8, 12, window_size.x);
    ImGui::PopID();
    ImGui::BeginChild("Edit", ImVec2(window_size.x, size_edit_h));
    {
        ImGui::Text("Camera");
        bool viewDirty = false;
        if (ImGui::RadioButton("Perspective", isPerspective)) isPerspective = true;
        ImGui::SameLine();
        if (ImGui::RadioButton("Orthographic", !isPerspective)) isPerspective = false;
        if (isPerspective)
        {
            ImGui::SliderFloat("Fov", &fov, 20.f, 110.f);
        }
        else
        {
            ImGui::SliderFloat("Ortho width", &viewWidth, 1, 20);
        }
        viewDirty |= ImGui::SliderFloat("Distance", &camDistance, 1.f, 10.f);
        ImGui::SliderInt("Gizmo count", &gizmoCount, 1, 4);
        if (viewDirty || firstFrame)
        {
            float eye[] = { cosf(camYAngle) * cosf(camXAngle) * camDistance, sinf(camXAngle) * camDistance, sinf(camYAngle) * cosf(camXAngle) * camDistance };
            float at[] = { 0.f, 0.f, 0.f };
            float up[] = { 0.f, 1.f, 0.f };
            LookAt(eye, at, up, cameraView);
            firstFrame = false;
        }
        ImGuiIO& io = ImGui::GetIO();
        ImGui::Text("X: %f Y: %f", io.MousePos.x, io.MousePos.y);
        if (ImGuizmo::IsUsing())
        {
            ImGui::Text("Using gizmo");
        }
        else
        {
            ImGui::Text(ImGuizmo::IsOver()?"Over gizmo":"");
            ImGui::SameLine();
            ImGui::Text(ImGuizmo::IsOver(ImGuizmo::TRANSLATE) ? "Over translate gizmo" : "");
            ImGui::SameLine();
            ImGui::Text(ImGuizmo::IsOver(ImGuizmo::ROTATE) ? "Over rotate gizmo" : "");
            ImGui::SameLine();
            ImGui::Text(ImGuizmo::IsOver(ImGuizmo::SCALE) ? "Over scale gizmo" : "");
        }
    }
    ImGui::EndChild();
    ImGui::BeginChild("Gizmo", ImVec2(window_size.x, size_zmo_h));
    for (int matId = 0; matId < gizmoCount; matId++)
    {
        ImGuizmo::SetID(matId);
        EditTransform(cameraView, cameraProjection, objectMatrix[matId]);
        ImGui::Separator();
    }

    ImGui::EndChild();
}
} // namespace IMGUIZMO_NAMESPACE
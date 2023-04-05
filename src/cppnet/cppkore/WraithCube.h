#pragma once

#include "Control.h"

namespace Forms
{
    // A class that handles a WraithCube
    class WraithCube : public Control
    {
    public:
        // -- WraithCube functions
        WraithCube();
        virtual ~WraithCube();

        void OnTimer(Message& Msg);

        // Loads a new icon from an HICON resource handle
        void LoadCubeIcon(HICON IconHandle);

        CreateParams GetCreateParams();

        // Sets up OpenGL properly
        virtual void OnRender();
        virtual void OnResize();
        virtual void OnHandleCreated();
        virtual void OnBackColorChanged();
        virtual void OnPaint(const std::unique_ptr<PaintEventArgs>& EventArgs);

        // Override WndProc for specific combo box messages.
        virtual void WndProc(Message& Msg);
        // Renders the cube
        void RenderCube();
        // Redraws the frame.
        void Redraw();

        // We must define event handlers here
        EventBase<void(*)(Control*)> Render;

    private:
        // Internal cached flags
        HGLRC _GLHandle;
        HDC _DCHandle;

        // -- Current texture
        uint32_t m_iTexture;

        UINT_PTR m_rTimer;

        // -- Values
        double RotateX = 0.0f;
        double RotateZ = 0.0f;
        double Distance = -5.2f;
    };
}
///////////////////////////////////////////////////////////////////////////////
//	Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//	See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#include "Prefix.h"
#include "Camera.h"
#include "Font.h"
#include "GraphicsDevice.h"
#include "LineRenderer.h"
#include "SkyRenderer.h"
#include "VoxelManager.h"
#include "VoxelRenderer.h"
#include "SceneManager.h"

const TCHAR* WindowClass = _T("Nyx");
const TCHAR* WindowName = _T("Nyx");
const DWORD WindowStyle = WS_OVERLAPPEDWINDOW ^ WS_SIZEBOX;
const size_t WindowWidth = 1024;
const size_t WindowHeight = 768;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (msg == WM_CLOSE)
	{
		PostQuitMessage(0);
		return 0;
	}
	else
	{
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}
}

bool PumpMessageQueue()
{
	MSG msg;
	while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
		{
			return false;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return true;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    HWND hwnd = 0;
	try
	{
        WNDCLASSEX wc;
        ZeroMemory(&wc, sizeof(wc));
        wc.cbSize = sizeof(wc);
        wc.hInstance = GetModuleHandle(0);
        wc.lpfnWndProc = WindowProc;
        wc.lpszClassName = WindowClass;
        wc.style = CS_OWNDC;
        WINCHECK(RegisterClassEx(&wc));

        RECT wndRect = {0, 0, WindowWidth, WindowHeight};
        WINCHECK(AdjustWindowRect(&wndRect, WindowStyle, false));

        WINCHECK(hwnd = CreateWindow(WindowClass,                   //  lpClassName 
                                     WindowName,                    //  lpWindowName,
                                     WindowStyle,                   //  dwStyle
                                     0,                             //  x
                                     0,                             //  y
                                     wndRect.right - wndRect.left,  //  nWidth
                                     wndRect.bottom - wndRect.top,  //  nHeight
                                     NULL,                          //  hWndParent
                                     NULL,                          //  hMenu
                                     GetModuleHandle(0),            //  hInstance
                                     NULL));                        //  lpParam 
        ShowWindow(hwnd, SW_SHOW);

        GraphicsDevice graphicsDevice(hwnd);
        SceneManager sceneManager(graphicsDevice);

        uint64_t lastTime, curTime, elapsedTime, pcFreq;
        const float TicksPerSecond = 60.0f;
        float ticks;

        QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&pcFreq));
        QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&lastTime));

        Camera camera;
        float3 position = float3(0, 0, 0);
        float roll = 0.0f,
              pitch = 0.0f;
        bool mouseDown;
        float speed = 0.35f;

        float4x4 projectionMatrix = float4x4::PerspectiveProjection(XM_PI / 3.0f,
                                                                    static_cast<float>(WindowWidth) / static_cast<float>(WindowHeight),
                                                                    0.1f,
                                                                    32000.0f);
        camera.SetProjectionMatrix(projectionMatrix);
        camera.SetPosition(float3(0, 1200.0f, 0));

        bool showGrid = false;

        while (PumpMessageQueue())
        {
            QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&curTime));
            elapsedTime = curTime - lastTime;
            lastTime = curTime;
            ticks = ((float)elapsedTime / (float)pcFreq) * TicksPerSecond;

            showGrid = GetAsyncKeyState(VK_CONTROL);

            if (GetAsyncKeyState(VK_LBUTTON) && GetFocus() == hwnd) {
                RECT wndRect;
                GetWindowRect(hwnd, &wndRect);
                LONG cx = wndRect.left + ((wndRect.right - wndRect.left) / 2),
                        cy = wndRect.top + ((wndRect.bottom - wndRect.top) / 2);
                POINT cursor;
                GetCursorPos(&cursor);
                LONG dx = cursor.x - cx, dy = cursor.y - cy;
                SetCursorPos(cx, cy);

                if (mouseDown) {
                    pitch += (static_cast<float>(dx) * 0.01f);
                    roll += (static_cast<float>(dy) * 0.01f);
                    if (roll < -XM_PIDIV2) {
                        roll = -XM_PIDIV2;
                    }
                    if (roll > XM_PIDIV2) {
                        roll = XM_PIDIV2;
                    }
                    float4 rot = float4::RotationQuaternion(roll, pitch, 0);
                    camera.SetRotation(rot);
                    ShowCursor(FALSE);
                }
                mouseDown = true;
            }
            else {
                ShowCursor(TRUE);
                mouseDown = false;
            }

            if (GetAsyncKeyState(VK_SHIFT))
            {
                speed = 10.0f;
            }
            else
            {
                speed = 0.45f;
            }
            if (GetAsyncKeyState('W')) {
                position += camera.GetForwardVector() * (speed * ticks);
            }
            if (GetAsyncKeyState('S')) {
                position -= camera.GetForwardVector() * (speed * ticks);
            }
            if (GetAsyncKeyState('A')) {
                position -= camera.GetRightVector() * (speed * ticks);
            }
            if (GetAsyncKeyState('D')) {
                position += camera.GetRightVector() * (speed * ticks);
            }
            camera.SetPosition(position);
            sceneManager.Update();
            sceneManager.SetCamera(camera);

            graphicsDevice.Begin();
            sceneManager.Draw();

            if (showGrid)
            {
                sceneManager.GetVoxelManager().DrawBoundingBoxes(camera);
            }
            graphicsDevice.End();
        }
    }
	catch (std::exception& e)
	{
        std::string msg = boost::diagnostic_information(e);
        OutputDebugStringA(msg.c_str());
        return -1;
	}

    if (hwnd)
    {
        DestroyWindow(hwnd);
    }

	return 0;
}


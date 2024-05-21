#pragma once

struct FrameContext
{
	ID3D12CommandAllocator* CommandAllocator;
	UINT64 FenceValue;
};

void SetupAndRun();
bool CreateDeviceD3D(HWND Window);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
void WaitForLastSubmittedFrame();
FrameContext* WaitForNextFrameResources();
LRESULT WINAPI WndProc(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam);

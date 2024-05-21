#pragma once

// STL
#include <cstdio>
#include <iostream>
#include <vector>
#include <format>
#include <algorithm>
#include <string>

// Windows
#define NOMINMAX
#include <Windows.h>

// dx
#include <d3d12.h>
#include <dxgi1_4.h>

#ifdef _DEBUG
#define DX12_ENABLE_DEBUG_LAYER
#include <dxgidebug.h>
#endif

// imgui
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx12.h>

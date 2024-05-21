// Precompiled headers
#include "Pch.h"

#include "UI.h"
#include "ErrorManager.h"
#include "Interpreter/Interpreter.h"
#include "Lexer/Lexer.h"
#include "Parser/NodeTypes.h"
#include "Parser/Parser.h"

static constexpr int NUM_FRAMES_IN_FLIGHT = 3;
static constexpr int NUM_BACK_BUFFERS = 3;

static FrameContext GFrameContext[NUM_FRAMES_IN_FLIGHT]{};
static UINT GFrameIndex = 0;
static ID3D12Device* GD3dDevice = nullptr;
static ID3D12DescriptorHeap* GD3dRtvDescriptorHeap = nullptr;
static ID3D12DescriptorHeap* GD3dSrvDescriptorHeap = nullptr;
static ID3D12CommandQueue* GD3dCommandQueue = nullptr;
static ID3D12GraphicsCommandList* GD3dCommandList = nullptr;
static ID3D12Fence* GFence = nullptr;
static HANDLE GFenceEvent = nullptr;
static UINT64 GFenceLastSignaledValue = 0;
static IDXGISwapChain3* GDxSwapChain = nullptr;
static HANDLE GSwapChainWaitableObject = nullptr;
static ID3D12Resource* GMainRenderTargetResource[NUM_BACK_BUFFERS] = {};
static D3D12_CPU_DESCRIPTOR_HANDLE GMainRenderTargetDescriptor[NUM_BACK_BUFFERS] = {};

// Main code
void SetupAndRun()
{
	// Create window class and register it
	const WNDCLASSEX WindowClass(sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"LiveCalc", nullptr);
	RegisterClassExW(&WindowClass);

	// Create window
	const HWND WindowHandle = CreateWindowW(
		WindowClass.lpszClassName,
		L"LiveCalculator",
		WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, 810, 260,
		NULL,
		NULL,
		WindowClass.hInstance,
		NULL
	);

	// Initialize Direct3D
	if (!CreateDeviceD3D(WindowHandle))
	{
		CleanupDeviceD3D();
		UnregisterClassW(WindowClass.lpszClassName, WindowClass.hInstance);

		return;
	}

	// Show the window
	ShowWindow(WindowHandle, SW_SHOWDEFAULT);
	UpdateWindow(WindowHandle);

	// Setup imgui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	// Setup imgui style
	ImGui::StyleColorsDark();

	// Setup renderer backends
	ImGui_ImplWin32_Init(WindowHandle);
	const auto CpuDescriptor = GD3dSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	const auto GpuDescriptor = GD3dSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	ImGui_ImplDX12_Init(GD3dDevice, NUM_FRAMES_IN_FLIGHT, DXGI_FORMAT_R8G8B8A8_UNORM, GD3dSrvDescriptorHeap, CpuDescriptor, GpuDescriptor);

	// Render/processing loop
	for (;;)
	{
		MSG Msg;
		bool Quit = false;

		while (PeekMessageW(&Msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&Msg);
			DispatchMessageW(&Msg);

			if (Msg.message == WM_QUIT)
			{
				Quit = true;
				break;
			}
		}

		if (Quit)
		{
			break;
		}

		// Start frame
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		// Draw LiveCalculator window
		{
			static std::string ResultString = "0";
			static std::string InputBuffer;

			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
			ImGui::Begin("LiveCalculator", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

			ImGui::Text("Input: ");
			ImGui::SameLine();

			if (ImGui::InputText("##Input", &InputBuffer))
			{
				if (!InputBuffer.empty())
				{
					// Run lexer
					std::vector<Token> Tokens = Lexer::GetTokens(InputBuffer);

					if (ErrorManager::CheckLastError())
					{
						// Run parser
						NodeBase* SyntaxTreeRoot = Parser::GetExpressionResult(Tokens);

						if (ErrorManager::CheckLastError())
						{
							// Run interpreter
							const Number Result = Interpreter::Visit(SyntaxTreeRoot);

							if (ErrorManager::CheckLastError())
							{
								if (Result.IsInt)
								{
									ResultString = std::format("{}", Result.IntValue);
								}
								else
								{
									ResultString = std::format("{}", Result.LongDoubleValue);
								}
							}
						}
					}
				}
				else
				{
					ResultString = "0";
				}
			}

			ImGui::SameLine();
			ImGui::Text("Result: %s", ResultString.c_str());

			// Print error details if error was found
			if (!ErrorManager::CheckLastError())
			{
				const Error* LastError = ErrorManager::GetLastError();
				ImGui::Text(
					"%s\n"
					"Error: %s -> %s",
					LastError->StringWithArrows(InputBuffer).c_str(),
					LastError->ErrorName.c_str(),
					LastError->Details.c_str());
			}

			ImGui::End();
		}

		// Render imgui frame
		ImGui::Render();

		FrameContext* NextFrameContext = WaitForNextFrameResources();
		const uint32_t BackBufferIndex = GDxSwapChain->GetCurrentBackBufferIndex();
		(void)NextFrameContext->CommandAllocator->Reset();

		D3D12_RESOURCE_BARRIER ResourceBarrier{};
		ResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		ResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		ResourceBarrier.Transition.pResource = GMainRenderTargetResource[BackBufferIndex];
		ResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		ResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		ResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		(void)GD3dCommandList->Reset(NextFrameContext->CommandAllocator, nullptr);
		GD3dCommandList->ResourceBarrier(1, &ResourceBarrier);

		// Render and present with DirectX
		constexpr float CLEAR_COLOR[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

		GD3dCommandList->ClearRenderTargetView(GMainRenderTargetDescriptor[BackBufferIndex], CLEAR_COLOR, 0, nullptr);
		GD3dCommandList->OMSetRenderTargets(1, &GMainRenderTargetDescriptor[BackBufferIndex], FALSE, nullptr);
		GD3dCommandList->SetDescriptorHeaps(1, &GD3dSrvDescriptorHeap);

		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), GD3dCommandList);

		ResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		ResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		GD3dCommandList->ResourceBarrier(1, &ResourceBarrier);
		(void)GD3dCommandList->Close();

		GD3dCommandQueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList* const*>(&GD3dCommandList));
		(void)GDxSwapChain->Present(1, 0);

		const uint64_t NextFenceValue = GFenceLastSignaledValue + 1;
		(void)GD3dCommandQueue->Signal(GFence, NextFenceValue);
		GFenceLastSignaledValue = NextFenceValue;
		NextFrameContext->FenceValue = NextFenceValue;
	}

	WaitForLastSubmittedFrame();

	// Cleanup
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	CleanupDeviceD3D();
	DestroyWindow(WindowHandle);
	UnregisterClassW(WindowClass.lpszClassName, WindowClass.hInstance);
}

// Helper functions

bool CreateDeviceD3D(const HWND Window)
{
	// Setup swap chain
	DXGI_SWAP_CHAIN_DESC1 SwapchainDescriptor;
	{
		ZeroMemory(&SwapchainDescriptor, sizeof(SwapchainDescriptor));
		SwapchainDescriptor.BufferCount = NUM_BACK_BUFFERS;
		SwapchainDescriptor.Width = 0;
		SwapchainDescriptor.Height = 0;
		SwapchainDescriptor.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		SwapchainDescriptor.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
		SwapchainDescriptor.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		SwapchainDescriptor.SampleDesc.Count = 1;
		SwapchainDescriptor.SampleDesc.Quality = 0;
		SwapchainDescriptor.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		SwapchainDescriptor.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		SwapchainDescriptor.Scaling = DXGI_SCALING_STRETCH;
		SwapchainDescriptor.Stereo = FALSE;
	}

	// Enable debug interface
#ifdef DX12_ENABLE_DEBUG_LAYER
	ID3D12Debug* Dx12Debug = nullptr;

	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&Dx12Debug))))
	{
		Dx12Debug->EnableDebugLayer();
	}
#endif

	// Create device
	if (D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&GD3dDevice)) != S_OK)
	{
		return false;
	}

	// [DEBUG] Setup debug interface to break on any warnings/errors
#ifdef DX12_ENABLE_DEBUG_LAYER
	if (Dx12Debug != nullptr)
	{
		ID3D12InfoQueue* InfoQueue = nullptr;

		(void)GD3dDevice->QueryInterface(IID_PPV_ARGS(&InfoQueue));
		(void)InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		(void)InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		(void)InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

		InfoQueue->Release();
		Dx12Debug->Release();
	}
#endif

	{
		D3D12_DESCRIPTOR_HEAP_DESC DescriptorHeapDesc;
		DescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		DescriptorHeapDesc.NumDescriptors = NUM_BACK_BUFFERS;
		DescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		DescriptorHeapDesc.NodeMask = 1;

		if (GD3dDevice->CreateDescriptorHeap(&DescriptorHeapDesc, IID_PPV_ARGS(&GD3dRtvDescriptorHeap)) != S_OK)
		{
			return false;
		}

		const size_t RtvDescriptorSize = GD3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		D3D12_CPU_DESCRIPTOR_HANDLE RtvCpuDescriptorHandle = GD3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

		for (auto& Descriptor : GMainRenderTargetDescriptor)
		{
			Descriptor = RtvCpuDescriptorHandle;
			RtvCpuDescriptorHandle.ptr += RtvDescriptorSize;
		}
	}

	{
		D3D12_DESCRIPTOR_HEAP_DESC DescriptorHeapDesc{};
		DescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		DescriptorHeapDesc.NumDescriptors = 1;
		DescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		if (GD3dDevice->CreateDescriptorHeap(&DescriptorHeapDesc, IID_PPV_ARGS(&GD3dSrvDescriptorHeap)) != S_OK)
		{
			return false;
		}
	}

	{
		D3D12_COMMAND_QUEUE_DESC CommandQueueDesc{};

		CommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		CommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		CommandQueueDesc.NodeMask = 1;

		if (GD3dDevice->CreateCommandQueue(&CommandQueueDesc, IID_PPV_ARGS(&GD3dCommandQueue)) != S_OK)
		{
			return false;
		}
	}

	for (auto& [CommandAllocator, Unused] : GFrameContext)
	{
		if (GD3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&CommandAllocator)) != S_OK)
		{
			return false;
		}
	}

	if (GD3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, GFrameContext[0].CommandAllocator, nullptr, IID_PPV_ARGS(&GD3dCommandList)) != S_OK ||
		GD3dCommandList->Close() != S_OK)
	{
		return false;
	}

	if (GD3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&GFence)) != S_OK)
	{
		return false;
	}

	GFenceEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);

	if (GFenceEvent == nullptr)
	{
		return false;
	}

	{
		IDXGIFactory4* DxgiFactory = nullptr;
		IDXGISwapChain1* SwapChain1 = nullptr;

		if (CreateDXGIFactory1(IID_PPV_ARGS(&DxgiFactory)) != S_OK)
		{
			return false;
		}

		if (DxgiFactory->CreateSwapChainForHwnd(GD3dCommandQueue, Window, &SwapchainDescriptor, nullptr, nullptr, &SwapChain1) != S_OK)
		{
			return false;
		}

		if (SwapChain1->QueryInterface(IID_PPV_ARGS(&GDxSwapChain)) != S_OK)
		{
			return false;
		}

		SwapChain1->Release();
		DxgiFactory->Release();
		(void)GDxSwapChain->SetMaximumFrameLatency(NUM_BACK_BUFFERS);
		GSwapChainWaitableObject = GDxSwapChain->GetFrameLatencyWaitableObject();
	}

	CreateRenderTarget();
	return true;
}

void CleanupDeviceD3D()
{
	CleanupRenderTarget();

	if (GDxSwapChain)
	{
		(void)GDxSwapChain->SetFullscreenState(false, nullptr);
		GDxSwapChain->Release();
		GDxSwapChain = nullptr;
	}

	if (GSwapChainWaitableObject != nullptr)
	{
		CloseHandle(GSwapChainWaitableObject);
	}

	for (auto& [CommandAllocator, Unused] : GFrameContext)
	{
		if (CommandAllocator)
		{
			CommandAllocator->Release();
			CommandAllocator = nullptr;
		}
	}

	if (GD3dCommandQueue)
	{
		GD3dCommandQueue->Release();
		GD3dCommandQueue = nullptr;
	}

	if (GD3dCommandList)
	{
		GD3dCommandList->Release();
		GD3dCommandList = nullptr;
	}

	if (GD3dRtvDescriptorHeap)
	{
		GD3dRtvDescriptorHeap->Release();
		GD3dRtvDescriptorHeap = nullptr;
	}

	if (GD3dSrvDescriptorHeap)
	{
		GD3dSrvDescriptorHeap->Release();
		GD3dSrvDescriptorHeap = nullptr;
	}

	if (GFence)
	{
		GFence->Release();
		GFence = nullptr;
	}

	if (GFenceEvent)
	{
		CloseHandle(GFenceEvent);
		GFenceEvent = nullptr;
	}

	if (GD3dDevice)
	{
		GD3dDevice->Release();
		GD3dDevice = nullptr;
	}

#ifdef DX12_ENABLE_DEBUG_LAYER
	IDXGIDebug1* DxgiDebug1 = nullptr;

	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&DxgiDebug1))))
	{
		(void)DxgiDebug1->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY);
		DxgiDebug1->Release();
	}
#endif
}

void CreateRenderTarget()
{
	for (size_t Index = 0; Index < NUM_BACK_BUFFERS; Index++)
	{
		ID3D12Resource* BackBuffer = nullptr;
		(void)GDxSwapChain->GetBuffer(static_cast<UINT>(Index), IID_PPV_ARGS(&BackBuffer));

		GD3dDevice->CreateRenderTargetView(BackBuffer, nullptr, GMainRenderTargetDescriptor[Index]);
		GMainRenderTargetResource[Index] = BackBuffer;
	}
}

void CleanupRenderTarget()
{
	WaitForLastSubmittedFrame();

	for (auto& Resource : GMainRenderTargetResource)
	{
		if (Resource)
		{
			Resource->Release();
			Resource = nullptr;
		}
	}
}

void WaitForLastSubmittedFrame()
{
	FrameContext* NextFrameContext = &GFrameContext[GFrameIndex % NUM_FRAMES_IN_FLIGHT];
	const uint64_t FenceValue = NextFrameContext->FenceValue;

	if (FenceValue == 0)
	{
		return; // No fence was signaled
	}

	NextFrameContext->FenceValue = 0;

	if (GFence->GetCompletedValue() >= FenceValue)
	{
		return;
	}

	(void)GFence->SetEventOnCompletion(FenceValue, GFenceEvent);
	WaitForSingleObject(GFenceEvent, INFINITE);
}

FrameContext* WaitForNextFrameResources()
{
	const uint32_t NextFrameIndex = GFrameIndex + 1;
	GFrameIndex = NextFrameIndex;

	HANDLE WaitableObjects[] = { GSwapChainWaitableObject, nullptr };
	DWORD NumWaitableObjects = 1;

	FrameContext* NextFrameContext = &GFrameContext[NextFrameIndex % NUM_FRAMES_IN_FLIGHT];

	if (const uint64_t FenceValue = NextFrameContext->FenceValue;
		FenceValue != 0) // No fence was signaled
	{
		NextFrameContext->FenceValue = 0;
		(void)GFence->SetEventOnCompletion(FenceValue, GFenceEvent);
		WaitableObjects[1] = GFenceEvent;
		NumWaitableObjects = 2;
	}

	WaitForMultipleObjects(NumWaitableObjects, WaitableObjects, TRUE, INFINITE);

	return NextFrameContext;
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
LRESULT WINAPI WndProc(const HWND Window, const UINT Message, const WPARAM WParam, const LPARAM LParam)
{
	if (ImGui_ImplWin32_WndProcHandler(Window, Message, WParam, LParam))
	{
		return true;
	}

	switch (Message)
	{
		case WM_SIZE:
		{
			if (GD3dDevice != nullptr && WParam != SIZE_MINIMIZED)
			{
				WaitForLastSubmittedFrame();
				CleanupRenderTarget();
				const HRESULT Result = GDxSwapChain->ResizeBuffers(0, LOWORD(LParam), HIWORD(LParam), DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT);
				assert(SUCCEEDED(Result) && "Failed to resize swapchain.");
				CreateRenderTarget();
			}

			return 0;
		}

		case WM_SYSCOMMAND:
		{
			if ((WParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			{
				return 0;
			}

			break;
		}

		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}

		default:
		{
			break;
		}
	}

	return DefWindowProcW(Window, Message, WParam, LParam);
}

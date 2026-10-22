#include <backends/imgui_impl_dx11.h>
#include <backends/imgui_impl_win32.h>
#include <d3d11.h>
#include <filesystem>
#include <gui/gui.hpp>
#include <windows.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);
namespace {
ID3D11Device *device = nullptr;
ID3D11DeviceContext *context = nullptr;
IDXGISwapChain *swap_chain = nullptr;
ID3D11RenderTargetView *target = nullptr;
bool make_target() {
    ID3D11Texture2D *buffer = nullptr;
    if (FAILED(swap_chain->GetBuffer(0, IID_PPV_ARGS(&buffer))))
        return false;
    auto result = device->CreateRenderTargetView(buffer, nullptr, &target);
    buffer->Release();
    return SUCCEEDED(result);
}
LRESULT WINAPI window_proc(HWND window, UINT message, WPARAM w, LPARAM l) {
    if (ImGui_ImplWin32_WndProcHandler(window, message, w, l))
        return true;
    if (message == WM_SIZE && device && w != SIZE_MINIMIZED) {
        if (target) {
            target->Release();
            target = nullptr;
        }
        if (SUCCEEDED(swap_chain->ResizeBuffers(0, LOWORD(l), HIWORD(l), DXGI_FORMAT_UNKNOWN, 0)))
            make_target();
        return 0;
    }
    if (message == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(window, message, w, l);
}
void cleanup() {
    if (target)
        target->Release();
    if (swap_chain)
        swap_chain->Release();
    if (context)
        context->Release();
    if (device)
        device->Release();
}
}
int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int) {
    ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEXW wc{sizeof(wc),
                   CS_CLASSDC,
                   window_proc,
                   0,
                   0,
                   instance,
                   nullptr,
                   LoadCursor(nullptr, IDC_ARROW),
                   nullptr,
                   nullptr,
                   L"TimewarpPreview",
                   nullptr};
    RegisterClassExW(&wc);
    RECT rect{0, 0, 772, 625};
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
    HWND window =
        CreateWindowW(wc.lpszClassName, L"timewarp - standalone preview", WS_OVERLAPPEDWINDOW, 100, 100,
                      rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr, instance, nullptr);
    DXGI_SWAP_CHAIN_DESC desc{};
    desc.BufferCount = 2;
    desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.OutputWindow = window;
    desc.SampleDesc.Count = 1;
    desc.Windowed = TRUE;
    desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    auto result =
        D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0,
                                      D3D11_SDK_VERSION, &desc, &swap_chain, &device, nullptr, &context);
    if (FAILED(result))
        result =
            D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0, nullptr, 0,
                                          D3D11_SDK_VERSION, &desc, &swap_chain, &device, nullptr, &context);
    if (FAILED(result) || !make_target()) {
        cleanup();
        DestroyWindow(window);
        return 1;
    }
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    gui::Interface ui;
    gui::State state;
    wchar_t executable[MAX_PATH];
    GetModuleFileNameW(nullptr, executable, MAX_PATH);
    auto font = std::filesystem::path(executable).parent_path() / "assets/fonts/PreviewSans.ttf";
    if (!ui.initialize(font.string().c_str())) {
        MessageBoxW(window, L"Missing or invalid preview fonts in assets/fonts", L"Preview error",
                    MB_ICONERROR);
        ImGui::DestroyContext();
        cleanup();
        DestroyWindow(window);
        return 1;
    }
    ImGui_ImplWin32_Init(window);
    ImGui_ImplDX11_Init(device, context);
    ShowWindow(window, SW_SHOWDEFAULT);
    UpdateWindow(window);
    bool running = true;
    while (running) {
        MSG msg;
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
            if (msg.message == WM_QUIT)
                running = false;
        }
        if (!running)
            break;
        if (IsIconic(window)) {
            Sleep(16);
            continue;
        }
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        ui.render(state);
        ImGui::Render();
        const float clear[] = {0.025f, 0.025f, 0.025f, 1};
        if (target) {
            context->OMSetRenderTargets(1, &target, nullptr);
            context->ClearRenderTargetView(target, clear);
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
            swap_chain->Present(1, 0);
        }
    }
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    cleanup();
    DestroyWindow(window);
    UnregisterClassW(wc.lpszClassName, instance);
    return 0;
}

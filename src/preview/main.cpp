#include <backends/imgui_impl_dx11.h>
#include <backends/imgui_impl_win32.h>
#include <d3d11.h>
#include <filesystem>
#include <fstream>
#include <limitless_gui/limitless.hpp>
#include <shellapi.h>
#include <string_view>
#include <vector>
#include <windows.h>
#include <wrl/client.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);
namespace {
ID3D11Device *device = nullptr;
ID3D11DeviceContext *context = nullptr;
IDXGISwapChain *swap_chain = nullptr;
ID3D11RenderTargetView *target = nullptr;
bool capture_frame(const std::filesystem::path &path) {
    Microsoft::WRL::ComPtr<ID3D11Texture2D> buffer, staging;
    if (FAILED(swap_chain->GetBuffer(0, IID_PPV_ARGS(&buffer))))
        return false;
    D3D11_TEXTURE2D_DESC desc{};
    buffer->GetDesc(&desc);
    desc.Usage = D3D11_USAGE_STAGING;
    desc.BindFlags = 0;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    desc.MiscFlags = 0;
    if (FAILED(device->CreateTexture2D(&desc, nullptr, &staging)))
        return false;
    context->CopyResource(staging.Get(), buffer.Get());
    D3D11_MAPPED_SUBRESOURCE mapped{};
    if (FAILED(context->Map(staging.Get(), 0, D3D11_MAP_READ, 0, &mapped)))
        return false;
    BITMAPFILEHEADER header{};
    BITMAPINFOHEADER info{};
    info.biSize = sizeof(info);
    info.biWidth = static_cast<LONG>(desc.Width);
    info.biHeight = -static_cast<LONG>(desc.Height);
    info.biPlanes = 1;
    info.biBitCount = 32;
    info.biCompression = BI_RGB;
    info.biSizeImage = desc.Width * desc.Height * 4;
    header.bfType = 0x4d42;
    header.bfOffBits = sizeof(header) + sizeof(info);
    header.bfSize = header.bfOffBits + info.biSizeImage;
    std::ofstream file(path, std::ios::binary);
    file.write(reinterpret_cast<const char *>(&header), sizeof(header));
    file.write(reinterpret_cast<const char *>(&info), sizeof(info));
    std::vector<unsigned char> row(desc.Width * 4);
    for (UINT y = 0; y < desc.Height; ++y) {
        const auto *source = static_cast<const unsigned char *>(mapped.pData) + y * mapped.RowPitch;
        for (UINT x = 0; x < desc.Width; ++x) {
            row[x * 4] = source[x * 4 + 2];
            row[x * 4 + 1] = source[x * 4 + 1];
            row[x * 4 + 2] = source[x * 4];
            row[x * 4 + 3] = 255;
        }
        file.write(reinterpret_cast<const char *>(row.data()), static_cast<std::streamsize>(row.size()));
    }
    context->Unmap(staging.Get(), 0);
    file.close();
    return !file.fail();
}
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
                   L"LimitlessGuiPreview",
                   nullptr};
    RegisterClassExW(&wc);
    RECT rect{0, 0, 772, 625};
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
    HWND window =
        CreateWindowW(wc.lpszClassName, L"limitless - standalone preview", WS_OVERLAPPEDWINDOW, 100, 100,
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
    limitless_gui::Interface ui;
    limitless_gui::State state;
    std::filesystem::path screenshot;
    int argc = 0;
    LPWSTR *arguments = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (arguments) {
        for (int arg = 1; arg + 1 < argc; ++arg) {
            const std::wstring_view option(arguments[arg]);
            if (option == L"--category") {
                const std::wstring_view value(arguments[++arg]);
                constexpr std::wstring_view categories[] = {L"settings", L"configs"};
                for (int i = 0; i < 2; ++i) {
                    if (value == categories[i]) {
                        state.category = static_cast<limitless_gui::Category>(i);
                        break;
                    }
                }
            } else if (option == L"--screenshot") {
                screenshot = arguments[++arg];
            }
        }
        LocalFree(arguments);
    }
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
    int frames = 0;
    int exit_code = 0;
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
            if (!screenshot.empty() && ++frames >= 60) {
                exit_code = capture_frame(screenshot) ? 0 : 1;
                running = false;
            }
            swap_chain->Present(1, 0);
        }
    }
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    cleanup();
    DestroyWindow(window);
    UnregisterClassW(wc.lpszClassName, instance);
    return exit_code;
}

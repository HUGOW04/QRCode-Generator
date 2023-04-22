#include <windows.h>
#include <string>
#include <vector>
#include "qrcodegen.hpp"




// Window class name and title
const char g_szClassName[] = "myQRCodeWindow";
const char g_szWindowTitle[] = "QR Code Generator";

// Window dimensions
const int g_windowWidth = 500;
const int g_windowHeight = 500;
const int g_inputBoxWidth = 350;
const int g_inputBoxHeight = 50;
const int g_qrCodeSize = 250;

// ID for the input box and the generate button
const int g_inputBoxId = 100;
const int g_generateButtonId = 101;
const int g_fileDialogButtonId = 102;


// Function to generate the QR code bitmap
HBITMAP GenerateQRCodeBitmap(const std::string& data)
{
    // Generate the QR code matrix
    qrcodegen::QrCode qr = qrcodegen::QrCode::encodeText(data.c_str(), qrcodegen::QrCode::Ecc::MEDIUM);

    // Get the QR code matrix size
    const int size = qr.getSize();

    // Calculate the size of each QR code module (square)
    const int moduleSize = g_qrCodeSize / size;

    // Create a bitmap for the QR code
    HDC hdcMem = CreateCompatibleDC(NULL);
    BITMAPINFO bmi = { 0 };
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = g_qrCodeSize;
    bmi.bmiHeader.biHeight = -g_qrCodeSize;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = g_qrCodeSize * g_qrCodeSize * 4;
    bmi.bmiHeader.biXPelsPerMeter = 0;
    bmi.bmiHeader.biYPelsPerMeter = 0;
    bmi.bmiHeader.biClrUsed = 0;
    bmi.bmiHeader.biClrImportant = 0;
    DWORD* pixels = nullptr;
    HBITMAP hBitmap = CreateDIBSection(hdcMem, &bmi, DIB_RGB_COLORS, (void**)&pixels, NULL, 0);
    SelectObject(hdcMem, hBitmap);

    // Fill the bitmap with white
    memset(pixels, 0xff, g_qrCodeSize * g_qrCodeSize * 4);

    // Draw the QR code modules (squares)
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            if (qr.getModule(x, y)) {
                int startX = x * moduleSize;
                int startY = y * moduleSize;
                int endX = startX + moduleSize;
                int endY = startY + moduleSize;
                for (int i = startY; i < endY; i++) {
                    for (int j = startX; j < endX; j++) {
                        pixels[i * g_qrCodeSize + j] = 0;
                    }
                }
            }
        }
    }

    // Cleanup
    DeleteDC(hdcMem);

    return hBitmap;
}

// Declare a global variable to store the QR code bitmap handle
HBITMAP g_qrCodeBitmap = NULL;


// Window procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        // Create the input box
        HWND inputBox = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "",
            WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
            (g_windowWidth - g_inputBoxWidth) / 2, 20, g_inputBoxWidth, g_inputBoxHeight,
            hwnd, (HMENU)g_inputBoxId, GetModuleHandle(NULL), NULL);

        // Create the generate button
        CreateWindow("BUTTON", "Generate", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            (g_windowWidth - g_inputBoxWidth) / 2, 90, g_inputBoxWidth, 30,
            hwnd, (HMENU)g_generateButtonId, GetModuleHandle(NULL), NULL);
        CreateWindow("BUTTON", "Select File", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            (g_windowWidth - g_inputBoxWidth) / 2, 120, g_inputBoxWidth, 30,
            hwnd, (HMENU)g_fileDialogButtonId, GetModuleHandle(NULL), NULL);
        break;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case g_generateButtonId:
        {
            // Get the data from the input box
            char data[1000];
            GetDlgItemText(hwnd, g_inputBoxId, data, sizeof(data));

            // Generate the QR code bitmap
            HBITMAP hBitmap = GenerateQRCodeBitmap(data);

            // Set the global QR code bitmap handle
            if (g_qrCodeBitmap != NULL)
            {
                DeleteObject(g_qrCodeBitmap);
            }
            g_qrCodeBitmap = hBitmap;

            // Refresh the window to display the QR code bitmap
            InvalidateRect(hwnd, NULL, TRUE);
            UpdateWindow(hwnd);
            break;
        }
        case g_fileDialogButtonId:
        {
            OPENFILENAME ofn;
            char szFileName[MAX_PATH] = "";
            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
            ofn.lpstrFile = szFileName;
            ofn.nMaxFile = MAX_PATH;
            ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
            ofn.lpstrDefExt = "txt";
            if (GetOpenFileName(&ofn))
            {
                // Display the selected file name in the input box
                SetDlgItemText(hwnd, g_inputBoxId, szFileName);
            }
            break;
        }

        }
        break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // Display the QR code bitmap
        if (g_qrCodeBitmap != NULL)
        {
            HDC hdcMem = CreateCompatibleDC(hdc);
            HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, g_qrCodeBitmap);

            // Scale the bitmap
            SetStretchBltMode(hdc, HALFTONE);
            StretchBlt(hdc, (g_windowWidth - g_qrCodeSize) / 2, 150, g_qrCodeSize, g_qrCodeSize, hdcMem, 0, 0, g_qrCodeSize, g_qrCodeSize, SRCCOPY);

            SelectObject(hdcMem, hOldBitmap);
            DeleteDC(hdcMem);
        }

        EndPaint(hwnd, &ps);
        break;
    }
    case WM_CLOSE:
        if (g_qrCodeBitmap != NULL)
        {
            DeleteObject(g_qrCodeBitmap);
        }
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}

// Entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Register the window class
    WNDCLASSEX wc;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = g_szClassName;
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    if (!RegisterClassEx(&wc))
    {
        MessageBox(NULL, "Window Registration Failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
    // Create the window
    HWND hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        g_szClassName,
        "QR Code Generator",
        WS_CAPTION | WS_SYSMENU | WS_VISIBLE | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, g_windowWidth, g_windowHeight,
        NULL, NULL, hInstance, NULL);

    if (hwnd == NULL)
    {
        MessageBox(NULL, "Window Creation Failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // Show the window
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

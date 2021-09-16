#ifndef UNICODE
#define UNICODE
#endif
#include <Windows.h>
#include <CL/cl.hpp>
#include <gl/GL.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#pragma comment (lib,"OpenCL.lib")
#pragma comment (lib,"opengl32.lib")
int* map;
cl::CommandQueue* CQ;
cl::Buffer* BA = NULL;
cl::Kernel* KN;
cl::Context* CT;
size_t MAX_WORK_ITEM_SIZES[3] = { 100,100 };
#define hight 100
#define width 150
#define WindowsHight 1000
#define WindowsWidth 1500
#define zoomX 1.015
#define zoomY 1.04
bool mouseleftbuttondown = 0;
bool setstatu = 0;
bool mapchange = 1;
void myPixelFormatSetupFunction(HDC hdc)
{
    PIXELFORMATDESCRIPTOR pfd = {
            sizeof(PIXELFORMATDESCRIPTOR),   // size of this pfd  
            1,                     // version number  
            PFD_DRAW_TO_WINDOW |   // support window  
            PFD_SUPPORT_OPENGL |   // support OpenGL  
            PFD_STEREO,            // 使用单重缓冲
            PFD_TYPE_RGBA,         // RGBA type  
            24,                    // 24-bit color depth  
            0, 0, 0, 0, 0, 0,      // color bits ignored  
            0,                     // no alpha buffer  
            0,                     // shift bit ignored  
            0,                     // no accumulation buffer  
            0, 0, 0, 0,            // accum bits ignored  
            32,                    // 32-bit z-buffer  
            0,                     // no stencil buffer  
            0,                     // no auxiliary buffer  
            PFD_MAIN_PLANE,        // main layer  
            0,                     // reserved  
            0, 0, 0                // layer masks ignored  
    };
    int  iPixelFormat = ChoosePixelFormat(hdc, &pfd);
    if (iPixelFormat == 0)
    {
        printf("%d", GetLastError());
        system("PAUSE");
        exit(-1);
    }
    if (SetPixelFormat(hdc, iPixelFormat, &pfd) == false)
    {
        printf("0x%X", GetLastError());
        system("PAUSE");
        exit(-5);
    }

}
LRESULT CALLBACK mainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    int mouseX, mouseY, i, o;
    int offsetX = 0, offsetY = 0;
    float psizey = (WindowsHight / (float)hight);
    float psizex = (WindowsWidth / (float)width);
    RECT rect;
    switch (uMsg)
    {

    case WM_CREATE:
        HDC      hdc;
        HGLRC    hglrc;
        hdc = GetDC(hwnd);
        myPixelFormatSetupFunction(hdc);
        if (hglrc = wglCreateContext(hdc)) {
            auto bHaveCurrentRC = wglMakeCurrent(hdc, hglrc);
        }
        GetWindowRect(hwnd, &rect);
        std::cout << rect.top << " " << rect.bottom << " " << rect.left << std::endl;
        glScaled(1.0 / (width * zoomX), 1. / (hight * zoomY), 1.0);
        std::cout << WindowsWidth / width;
        glViewport(-WindowsWidth, -WindowsHight, WindowsWidth * 2, (WindowsHight) * 2);
        glPointSize(1);
        break;
    case WM_PAINT:
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        for (int i = 0; i < hight; i++)
        {
            for (int o = 0; o < width; o++)
            {
                if (map[i * width + o])
                    glRectf(o, i, o + 1, i + 1);
            }
        }
        glFlush();
        break;
    case WM_LBUTTONDOWN:
        mouseleftbuttondown = 1;
        mouseX = LOWORD(lParam) * zoomX;
        mouseY = HIWORD(lParam) * zoomY;
        i = hight - (mouseY / psizey);
        o = mouseX / psizex;
        setstatu = map[i * width + o] == 0 ? 1 : 0;
        break;
    case WM_LBUTTONUP:
        if (mouseleftbuttondown == 1)
        {
            mouseleftbuttondown = 0;
            mouseX = LOWORD(lParam) * zoomX;
            mouseY = HIWORD(lParam) * zoomY;
            i = hight - (mouseY / psizey);
            o = mouseX / psizex;
            if (i < hight && o < width)
                map[i * width + o] = setstatu;
            mapchange = 1;
            SendMessage(hwnd, WM_PAINT, wParam, lParam);
        }
        std::cout << i << "," << o << std::endl;
        break;
    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_RIGHT:
            int err;
            if (mapchange == true)
            {
                if (BA != nullptr)
                    delete BA;
                BA = new cl::Buffer(*CT, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(int) * hight * width, map, &err);
                err = KN->setArg(0, *BA);
                if (err != CL_SUCCESS)
                {
                    std::cout << "set Arg err" << err << std::endl;
                }
                mapchange = 0;
            }
            for (int wt = 0; wt < width; wt += MAX_WORK_ITEM_SIZES[0])
            {
                for (int ht = 0; ht < hight; ht += MAX_WORK_ITEM_SIZES[1])
                {
                    int hi, wi;
                    if (wt + MAX_WORK_ITEM_SIZES[0] > width)
                        wi = width % MAX_WORK_ITEM_SIZES[0];
                    else
                        wi = wt + MAX_WORK_ITEM_SIZES[0];
                    if (ht + MAX_WORK_ITEM_SIZES[1] > hight)
                        hi = hight % MAX_WORK_ITEM_SIZES[1];
                    else
                        hi = ht + MAX_WORK_ITEM_SIZES[1];
                    err = CQ->enqueueNDRangeKernel(*KN, cl::NDRange(wt, ht), cl::NDRange(wi, hi)); if (err != CL_SUCCESS)std::cout << "NDRange ERR" << err << std::endl;
                }
            }
            err = CQ->enqueueReadBuffer(*BA, CL_TRUE, 0, hight * width * sizeof(int), map); if (err != CL_SUCCESS)std::cout << "ReadBuffer ERR" << err << std::endl;
            SendMessage(hwnd, WM_PAINT, wParam, lParam);
            break;
        case 'C':
            for (int l = 0; l < hight * width; l++)
            {
                map[l] = 0;
            }
            mapchange = 1;
            SendMessage(hwnd, WM_PAINT, wParam, lParam);
            break;
        default:
            break;
        }
        break;
    case WM_MOUSEMOVE:
        if (mouseleftbuttondown) {
            mouseX = LOWORD(lParam) * zoomX;
            mouseY = HIWORD(lParam) * zoomY;
            i = hight - (mouseY / psizey);
            o = mouseX / psizex;
            if (i < hight && o < width)
                map[i * width + o] = setstatu;
            mapchange = 1;
            SendMessage(hwnd, WM_PAINT, wParam, lParam);
        }
        break;
    default:
        break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int main()
{
    using namespace std;
    using namespace cl;
    int err;
    vector<Platform> vP;
    Platform::get(&vP);
    for (auto& i : vP)
    {
        cout << i.getInfo<CL_PLATFORM_VERSION>() << endl;
    }
    Platform platfrom = vP[0];
    vector<Device> vD;
    platfrom.getDevices(CL_DEVICE_TYPE_ALL, &vD);
    vD[0].getInfo(CL_DEVICE_MAX_WORK_ITEM_SIZES, &MAX_WORK_ITEM_SIZES);
    Context context(vD, 0, 0, 0, &err);
    if (err != CL_SUCCESS)cout << "Context ERR" << endl;
    CommandQueue cQ(context, vD[0], 0, &err);
    if (err != CL_SUCCESS)cout << "CQ ERR" << endl;
    string line, kn_code;
    ifstream in("add.cl", ios::in);
    if (in.is_open())
    {
        while (getline(in, line))
        {
            kn_code += line + '\n';
        }
    }
    cout << kn_code << endl;
    Program vecAdd(context, kn_code, true, &err);
    if (err != CL_SUCCESS)cout << "Creat Program ERR" << err << endl;
    Kernel kn(vecAdd, "yunpaozidongji", &err);
    if (err != CL_SUCCESS)cout << "Creat Kenel ERR" << err << endl;

    int* a = new int[hight * width];
    //初始化；
    for (int i = 0; i < hight * width; i++)
    {
        a[i] = 0;
    }
    int b = width;
    int c = hight;
    Buffer bb(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int), &b, &err); if (err != CL_SUCCESS)std::cout << "set Arg err" << err << std::endl;
    Buffer bc(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int), &c, &err); if (err != CL_SUCCESS)std::cout << "set Arg err" << err << std::endl;
    KN = &kn;
    err = KN->setArg(1, bb); if (err != CL_SUCCESS)std::cout << "setArg ERR" << err << std::endl;
    err = KN->setArg(2, bc); if (err != CL_SUCCESS)std::cout << "setArg ERR" << err << std::endl;
    CT = &context;
    CQ = &cQ;
    map = a;
    WNDCLASSA wclass = { 0 };
    wclass.lpfnWndProc = mainWindowProc;
    wclass.hInstance = GetModuleHandle(0);
    wclass.lpszClassName = (LPCSTR)TEXT("Coire");
    RegisterClass((const WNDCLASSW*)&wclass);
    HWND hwnd = CreateWindow(
        TEXT("Coire"),
        TEXT("元胞自动机"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        WindowsWidth, WindowsHight,
        NULL,
        NULL,
        wclass.hInstance,
        NULL
    );
    if (hwnd == 0)
    {
        return GetLastError();
    }
    ShowWindow(hwnd, 5);
    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    delete[] a;
}
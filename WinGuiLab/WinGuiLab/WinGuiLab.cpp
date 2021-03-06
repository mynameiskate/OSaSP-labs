// WinGuiLab.cpp : Defines the entry point for the application.
//

#include "windows.h"
#include "WinGuiLab.h"

#define MAX_LOADSTRING 100
#define MASK 0xFFFFFF
#define COLOR_BACKGROUND COLOR_MENU
#define MOVE_SIZE 20
#define BMP_BORDER_SIZE 30

#define DIRECTION_NONE 0x00
#define DIRECTION_UP 0x01
#define DIRECTION_RIGHT 0x02
#define DIRECTION_LEFT 0x03
#define DIRECTION_DOWN 0x04

#define ELLIPSE_HEIGHT 0xFF
#define ELLIPSE_WIDTH 0xFF
#define ELLIPSE_COLOR 0x00332211

#define IDT_SPRITE_TIMER 0xFF
#define OBJECT_TIMER_INTERVAL 20

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

POINT ptMouseOffset = { 0, 0 };
POINT ptCurPos = { 0, 0 };
POINT ptMaxPos = { 0, 0 };
HBITMAP hBitmap;
BOOL isVertical = true;
BOOL isSpriteSelected = false;
BOOL isImgLoaded = true;
BOOL isSpeedAdded = false;
INT objDirection = DIRECTION_NONE;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

VOID PaintObject(HWND hWnd);
BOOL IsMouseOverSprite(POINT ptMousePos, POINT ptCurPos, BITMAP bmObj);
VOID MoveRight(int rightBorder, int bmWidth);
VOID MoveDown(int rightBorder, int bmWidth);
VOID MoveLeft();
VOID MoveUp();
VOID CALLBACK AddSpeed(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
VOID RecreateObject(HWND hWnd);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINGUILAB, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINGUILAB));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}


//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINGUILAB));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)COLOR_BACKGROUND;
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINGUILAB);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        BITMAP bm;
        RECT wndRect;

        case WM_CREATE:
        {
            hBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP1));
            GetObject(hBitmap, sizeof(bm), &bm); //load bitmap info into buffer
            break;
        }
        case WM_LBUTTONDOWN:
        {
            POINT ptMousePos = { LOWORD(lParam), HIWORD(lParam) };
            GetObject(hBitmap, sizeof(BITMAP), &bm);

            if (IsMouseOverSprite(ptMousePos, ptCurPos, bm))
            {
                ptMouseOffset = { abs(ptMousePos.x - ptCurPos.x), abs(ptMousePos.y - ptCurPos.y) };
                isSpriteSelected = true;
            }
            break;
        }
        case WM_LBUTTONUP:
        {
            isSpriteSelected = false;
            break;
        }
        case WM_MOUSEMOVE:
        {
            GetObject(hBitmap, sizeof(BITMAP), &bm);
            if (isSpriteSelected)
            {
                RECT rectWndParams;
                GetClientRect(hWnd, &rectWndParams);

                int ptX = LOWORD(lParam) - ptMouseOffset.x;
                int ptY = HIWORD(lParam) - ptMouseOffset.y;

                if (ptX < 0)
                {
                    ptCurPos.x = 0;
                }
                else if (ptX + bm.bmWidth > rectWndParams.right)
                {
                    ptCurPos.x = rectWndParams.right - bm.bmWidth;
                }
                else
                {
                    ptCurPos.x = ptX;
                }

                if (ptY < 0)
                {
                    ptCurPos.y = 0;
                }
                else if (ptY + bm.bmHeight > rectWndParams.bottom)
                {
                    ptCurPos.y = rectWndParams.bottom - bm.bmHeight;
                }
                else
                {
                    ptCurPos.y = ptY;
                }

                InvalidateRgn(hWnd, NULL, TRUE);
            }

            break;
        }
        case WM_MOUSEWHEEL:
        {
            GetObject(hBitmap, sizeof(BITMAP), &bm);
            GetClientRect(hWnd, &wndRect);

            const int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
            const int fwKeys = GET_KEYSTATE_WPARAM(wParam);

            if (fwKeys == MK_SHIFT)
            {
                if (zDelta > 0)
                {
                    MoveRight(wndRect.right, bm.bmWidth);
                }
                if (zDelta < 0)
                {
                    MoveLeft();
                }
            }
            else
            {
                if (zDelta > 0)
                {
                    MoveUp();
                }
                if (zDelta < 0)
                {
                    MoveDown(wndRect.bottom, bm.bmHeight);
                }
            }
            InvalidateRect(hWnd, NULL, TRUE);
            break;
        }
        case WM_KEYDOWN:
        {
            GetObject(hBitmap, sizeof(BITMAP), &bm);
            GetClientRect(hWnd, &wndRect);

            int wmId = LOWORD(wParam);
            switch (wmId)
            {
                case VK_DOWN:
                {
                    MoveDown(wndRect.bottom, bm.bmHeight);
                    InvalidateRect(hWnd, NULL, TRUE);

                    break;
                }
                case VK_UP:
                {
                    MoveUp();
                    InvalidateRect(hWnd, NULL, TRUE);

                    break;
                }
                case VK_LEFT:
                {
                    MoveLeft();
                    InvalidateRect(hWnd, NULL, TRUE);

                    break;
                }
                case VK_RIGHT:
                {
                    MoveRight(wndRect.right, bm.bmWidth);
                    InvalidateRect(hWnd, NULL, TRUE);

                    break;
                }
                case VK_SPACE:
                {
                    if (isSpeedAdded)
                    {
                        isSpeedAdded = false;
                        KillTimer(hWnd, IDT_SPRITE_TIMER);
                    }
                    else
                    {
                        isSpeedAdded = true;
                        SetTimer(hWnd, IDT_SPRITE_TIMER, OBJECT_TIMER_INTERVAL, AddSpeed);
                    }
                }
            }
            break;
        }
        case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
                case IDM_ABOUT:
                    DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                    break;
                case IDM_EXIT:
                    DestroyWindow(hWnd);
                    break;
                case IDM_PICKOBJECTTYPE_BITMAP:
                    isImgLoaded = true;
                    RecreateObject(hWnd);
                    break;
                case ID_PICKOBJECTTYPE_ELLIPSE:
                    isImgLoaded = false;
                    RecreateObject(hWnd);
                    break;
                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;
        }
        case WM_PAINT:
        {
            PaintObject(hWnd);
            break;
        }
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            break;
        }
        case WM_GETMINMAXINFO:
        {
            GetObject(hBitmap, sizeof(BITMAP), &bm);
            LPMINMAXINFO lpInfo = (LPMINMAXINFO)lParam;

            if (isImgLoaded)
            {
                lpInfo->ptMinTrackSize.x = ptCurPos.x + bm.bmWidth;
                lpInfo->ptMinTrackSize.y = ptCurPos.y + bm.bmHeight + BMP_BORDER_SIZE * 2;
            }
            else
            {
                lpInfo->ptMinTrackSize.x = ptCurPos.x + ELLIPSE_WIDTH;
                lpInfo->ptMinTrackSize.y = ptCurPos.y + ELLIPSE_HEIGHT + BMP_BORDER_SIZE * 2;;
            }
            break;
        }
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

BOOL IsMouseOverSprite(POINT ptMousePos, POINT ptCurPos, BITMAP bmObj)
{
    return ((ptMousePos.x < ptCurPos.x + bmObj.bmWidth - BMP_BORDER_SIZE)
        && (ptMousePos.x > ptCurPos.x + BMP_BORDER_SIZE)
        && (ptMousePos.y < ptCurPos.y + bmObj.bmHeight - BMP_BORDER_SIZE)
        && (ptMousePos.y > ptCurPos.y + BMP_BORDER_SIZE));
}

VOID PaintObject(HWND hWnd)
{
    HBITMAP hPrevBmp;
    PAINTSTRUCT ps;
    HDC hWndDc;
    HDC hMemDc;
    BITMAP bmp;
    RECT wndRect;

    GetClientRect(hWnd, &wndRect);
    hWndDc = BeginPaint(hWnd, &ps);

    if (isImgLoaded)
    {
        GetObject(hBitmap, sizeof(BITMAP), &bmp);

        hMemDc = CreateCompatibleDC(hWndDc);
        hPrevBmp = (HBITMAP)SelectObject(hMemDc, hBitmap);
        TransparentBlt(hWndDc, ptCurPos.x, ptCurPos.y, bmp.bmWidth, bmp.bmHeight, hMemDc,
            0, 0, bmp.bmWidth, bmp.bmHeight, MASK);
        SelectObject(hMemDc, hPrevBmp);
        DeleteDC(hMemDc);
        DeleteObject(hPrevBmp);
    }
    else
    {
        HBRUSH hBr = CreateSolidBrush(ELLIPSE_COLOR);
        HBRUSH hOldBr = (HBRUSH)SelectObject(hWndDc, hBr);

        Ellipse(hWndDc, ptCurPos.x, ptCurPos.y, ptCurPos.x + ELLIPSE_WIDTH, ptCurPos.y + ELLIPSE_HEIGHT);

        SelectObject(hWndDc, hOldBr);
        DeleteObject(hBr);
    }

    EndPaint(hWnd, &ps);
}

VOID CALLBACK AddSpeed(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    BITMAP bm;
    RECT wndRect;

    GetObject(hBitmap, sizeof(BITMAP), &bm);
    GetClientRect(hWnd, &wndRect);

    switch (objDirection) {
        case DIRECTION_UP:
        {
            MoveUp();
            break;
        }
        case DIRECTION_DOWN:
        {
            MoveDown(wndRect.bottom, bm.bmHeight);
            break;
        }
        case DIRECTION_LEFT:
        {
            MoveLeft();
            break;
        }
        case DIRECTION_RIGHT:
        {
            MoveRight(wndRect.right, bm.bmWidth);
            break;
        }
        break;
    }

    InvalidateRect(hWnd, &wndRect, TRUE);
}


VOID MoveRight(int rightBorder, int bmWidth)
{
    int posLimit = rightBorder - bmWidth;
    int nextPosition = ptCurPos.x + MOVE_SIZE;

    if (nextPosition > posLimit)
    {
        ptCurPos.x = posLimit;
        objDirection = DIRECTION_LEFT;
    }
    else
    {
        ptCurPos.x = nextPosition;
        objDirection = DIRECTION_RIGHT;
    }
}

VOID MoveLeft()
{
    int posLimit = 0;
    int nextPosition = ptCurPos.x - MOVE_SIZE;

    if (nextPosition < posLimit)
    {
        ptCurPos.x = posLimit;
        objDirection = DIRECTION_RIGHT;
    }
    else
    {
        ptCurPos.x = nextPosition;
        objDirection = DIRECTION_LEFT;
    }
}

VOID MoveDown(int bottomBorder, int bmHeight)
{
    int posLimit = bottomBorder - bmHeight;
    int nextPosition = ptCurPos.y + MOVE_SIZE;

    if (nextPosition > posLimit)
    {
        ptCurPos.y = posLimit;
        objDirection = DIRECTION_UP;
    }
    else
    {
        ptCurPos.y = nextPosition;
        objDirection = DIRECTION_DOWN;
    }
}

VOID RecreateObject(HWND hWnd)
{
    objDirection = DIRECTION_NONE;
    InvalidateRect(hWnd, NULL, TRUE);
}

VOID MoveUp()
{
    int posLimit = 0;
    int nextPosition = ptCurPos.y - MOVE_SIZE;

    if (nextPosition < posLimit)
    {
        ptCurPos.y = posLimit;
        objDirection = DIRECTION_DOWN;
    }
    else
    {
        ptCurPos.y = nextPosition;
        objDirection = DIRECTION_UP;
    }
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        case WM_INITDIALOG:
            return (INT_PTR)TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
            {
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
            break;
    }
    return (INT_PTR)FALSE;
}

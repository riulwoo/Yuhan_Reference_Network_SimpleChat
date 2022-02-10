﻿// sockettest.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "sockettest.h"
#include <WinSock2.h>                           //window 환경에서 소켓 프로그래밍 관련 기능을 제공

#pragma comment(lib, "ws2_32.lib")            

#pragma warning(disable:4996);

#define SC_WIDTH 1425                        //클라이언트 영역 넓이를 나타냅니다.
#define SC_HEIGHT 751                        //클라이언트 영역 높이를 나타냅니다.

#define MAX 512
#define MAX_LOADSTRING 100
#define IDM_BTN_ServSTART 101
#define IDM_BTN_ServCLOSE 102

#define TIMER_ID_RECVMS 1

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SOCKETTEST, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SOCKETTEST));

    MSG msg;

    // 기본 메시지 루프입니다:
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
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SOCKETTEST));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_SOCKETTEST);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

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
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//

WSADATA wsaData;
SOCKET hServSock, hClntSock[MAX];
SOCKADDR_IN servAddr, clntAddr[MAX];
bool servRunning = false;
fd_set set, cpset;
int fd_max, fd_num, chk_conn;

WCHAR buf[MAX] = {};
WCHAR buf2[MAX] = {};

DWORD WINAPI runServ(LPVOID Param)
{
    HWND hWnd;
    hWnd = (HWND)Param;
    int portNumber = 10000;
    int szClntAddr;
    int i, j = 0;
    timeval timeout;
    HDC hdc = GetDC(hWnd);
    SetBkMode(hdc, TRANSPARENT);
    InvalidateRect(hWnd, NULL, 1);

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        MessageBox(hWnd, L"winsock error!", L"에러", NULL);
    }

    hServSock = socket(PF_INET, SOCK_STREAM, 0);
    if (hServSock == INVALID_SOCKET)
    {
        MessageBox(hWnd, L"socket() error!", L"에러", NULL);
        WSACleanup();
    }

    TextOut(hdc, 10, 10 + j, L"Successfully created socket.", lstrlenW(L"Successfully created socket."));
    j += 20;
    InvalidateRect(hWnd, NULL, 0);

    memset(&servAddr, 0x00, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(portNumber);

    if (bind(hServSock, (sockaddr*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
    {
        MessageBox(hWnd, L"bind() 오류", L"에러", NULL);
        closesocket(hServSock);
        WSACleanup();
    }
    else {
        TextOut(hdc, 10, 10 + j, L"Bind succeed.", lstrlenW(L"Bind succeed."));
        j += 20;
        InvalidateRect(hWnd, NULL, 0);
    }

    listen(hServSock, 10);
    
    TextOut(hdc, 10, 10 + j, L"Listening...", lstrlenW(L"Listening..."));
    j += 20;
    InvalidateRect(hWnd, NULL, 0);

    FD_ZERO(&set);
    FD_SET(hServSock, &set);

    TextOut(hdc, 10, 10 + j, L"Server is running...", lstrlenW(L"Server is running..."));
    j += 20;
    InvalidateRect(hWnd, NULL, 0);

    while(1)
    {
        if (servRunning == false)
            break;

        cpset = set;
        timeout.tv_sec = 5;
        timeout.tv_usec = 5000;

        fd_num = select(set.fd_count, &cpset, 0, 0, &timeout);
        
        if (fd_num == -1) 
            break;
        else if (fd_num == 0)
            continue;

        for (i = 0; i < set.fd_count; i++) 
        {
            if (FD_ISSET(set.fd_array[i], &cpset))  //배열 내 소켓에서 변화 감지
            {
                if (set.fd_array[i] == hServSock)   //해당 소켓이 서버 소켓이라면 연결
                {
                    szClntAddr = sizeof(clntAddr);
                    hClntSock[i] = accept(hServSock, (SOCKADDR*)&clntAddr, &szClntAddr);
                    FD_SET(hClntSock[i], &set);
                    if (set.fd_count < i)
                        set.fd_count = i;
                    wsprintf(buf, L"Connected Client: %d", hClntSock[i]);
                    if (hClntSock[i] != -1) {
                        TextOut(hdc, 10, 10 + j, buf, lstrlenW(buf));
                        j += 20;
                    }
                    InvalidateRect(hWnd, NULL, 0);
                }
                else
                {
                    chk_conn = recv(set.fd_array[i], (char*)buf, MAX, 0);

                    if (chk_conn <= 0)
                    {
                        wsprintf(buf, L"Disconnected Client: %d", set.fd_array[i]);
                        closesocket(set.fd_array[i]);
                        FD_CLR(set.fd_array[i], &set);
                        TextOut(hdc, 10, 10 + j, buf, lstrlenW(buf));
                        j += 20;
                        InvalidateRect(hWnd, NULL, 0);
                    }
                    else
                    {
                        wsprintf(buf2, L"%d         : ", set.fd_array[i]);
                        wcscat(buf2, buf);
                        TextOut(hdc, 10, 10 + j, buf2, lstrlenW(buf2));
                        j += 20;
                        InvalidateRect(hWnd, NULL, 0);
                        //모든 클라이언트에 재전송
                    }
                }
            }
        }
    }
    TextOut(hdc, 10, 10 + j, L"Server Terminated.", lstrlenW(L"Server Terminated."));
    j += 20;
    InvalidateRect(hWnd, NULL, 0);
    ReleaseDC(hWnd, hdc);
    closesocket(hServSock);
    WSACleanup();
    ExitThread(0);
    return 0;
}




LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        CreateWindow(L"button", L"Launch Server", WS_CHILD | WS_VISIBLE, 500, 200, 200, 60, hWnd, (HMENU)IDM_BTN_ServSTART, hInst, NULL);
        CreateWindow(L"button", L"Terminate Server", WS_CHILD | WS_VISIBLE, 850, 200, 200, 60, hWnd, (HMENU)IDM_BTN_ServCLOSE, hInst, NULL);
    }
        break;

    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 메뉴 선택을 구문 분석합니다:
            switch (wmId)
            {
            case IDM_BTN_ServSTART:
                if (servRunning == false)
                {
                    CreateThread(NULL, 0, runServ, (LPVOID)hWnd, 0, NULL);
                    servRunning = true;
                }
                break;

            case IDM_BTN_ServCLOSE:
                if (servRunning == true)
                {
                    servRunning = false;
                    //WSACleanup();
                    closesocket(hServSock);
                    MessageBox(hWnd, L"Server terminated.", L"서버 종료", NULL);
                }
                break;

            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;

            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;

            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;

    case WM_TIMER:
    {
    /*
        switch (wParam)
        {
        case TIMER_ID_RECVMS:
        {
            
            break;
        }}
    */
    }
        break;

    case WM_LBUTTONDOWN:
    {

    }
    break;

    case WM_RBUTTONDOWN:
    {

    }
        break;
        /*
    case WM_USER+1:
    {
        if (servRunning == true && recv(hClntSock, (char*)buf, MAX, 0))
        MessageBox(hWnd, buf, buf, NULL);
    }   
    break;
    */
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            int i;
            // TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...
            EndPaint(hWnd, &ps);
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
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

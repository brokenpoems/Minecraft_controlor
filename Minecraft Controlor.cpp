// Minecraft Controlor.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "Minecraft Controlor.h"
#include "FMOD/fmod.hpp"
#include "FMOD/fmod_errors.h"

// Fmod类库
#pragma comment(lib,"fmod_vc.lib")

// 宏
#define MAX_LOADSTRING 100                                        // 最大载入字符串
#define WS_CHILDREN    WS_CHILD|WS_VISIBLE                        // 子窗口样式
#define MB_ERROR       MB_OK | MB_ICONERROR | MB_DEFBUTTON1       // 消息框错误样式
#define MB_ATTENTION   MB_OK | MB_DEFBUTTON1                      // 消息框提示样式
#define MBBOXERR(x)    MessageBox(NULL, x, L"错误", MB_ERROR)     // 错误消息框
#define MBBOXATT(x)    MessageBox(NULL, x, L"提示", MB_ATTENTION) // 提示消息框

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
WCHAR szCFGNAME[2][MAX_LOADSTRING];             // 配置文件名
wifstream fin;                                  // 宽字符文件输入流对象
wofstream fout;                                 // 宽字符文件输出流对象
wstringstream strio;                            // 宽字符串输入输出流对象
locale loc("chs");                              // 语言
HWND hCombobox,                                 // 组合框窗口句柄
hMinecraft,                                     // Minecraft窗口句柄
hButton,                                        // 删除按钮窗口句柄
hProcess;                                       // 当前窗口句柄
FMOD_RESULT result;                             // FMOD函数执行结果
FMOD::System* System = NULL;                    // FMOD声音系统
FMOD::Sound* Sound;                             // FMOD声音实例
FMOD::Channel* channel;                         // FMOD声音通道
FMOD::ChannelGroup* channelgroup;               // FMOD声音通道组
HANDLE hMutex;                                  // 互斥锁
int Comboboxlong = 0;                           // 组合框大小
wstring NowChooce;                              // 现在选择版本
int cxchar = 0,                                 // 字符宽
cychar = 0,                                     // 字符高
cxscreen = 0,                                   // 屏幕宽
cyscreen = 0,                                   // 屏幕高
Time = 0,                                       // 程序运行时间(s)
init= 0,                                        // 初始化对话框
music = 0;                                      // 正在播放音乐ID
bool ishide = 0,                                // 是否为隐藏状态
isPause = 0;                                    // 音乐播放状态
HMENU hMenu;                                    // 菜单句柄
const int wide = 320,                           // 程序窗口宽
high = 190;                                     // 程序窗口高

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE);                 // 注册窗口类      
BOOL                InitInstance(HINSTANCE, int);               // 初始化实例                          
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);        // 主窗口消息处理                          
INT_PTR CALLBACK    DialogDefault(HWND, UINT, WPARAM, LPARAM);  // 对话框默认消息处理
INT_PTR CALLBACK    Insert(HWND, UINT, WPARAM, LPARAM);         // 版本插入对话框消息处理       
INT_PTR CALLBACK    hotkeyset(HWND, UINT, WPARAM, LPARAM);      // 热键设置对话框消息处理
INT_PTR CALLBACK    SetVolume(HWND, UINT, WPARAM, LPARAM);      // 声音设置对话框消息处理
BOOL                ReleaseRes(wstring, WORD, wstring);         // 释放资源                         
BOOL                HotkeyloadFlie();                           // 热键读入                               
BOOL                HotkeysaveFlie();                           // 热键保存                          
BOOL                HotkeyRegister(HWND);                       // 注册热键
BOOL                FillComboBox();                             // 填充组合框
wstring             WriteTimeString();                          // 计算时间
BOOL                CompareVersion(wstring,wstring);            // 比较Minecraft版本(待Minecraft1.16正式版出了再做)
//热键类
class HOTKEY {
    protected:
    bool SHIFT;// Shift键是否按下
    bool CTRL; // Ctrl键是否按下
    bool ALT;  // Alt键是否按下
    bool EXT;  // 是否为扩展键(VK_UP,VK_DOWN,VK_LEFT,VK_RIGHT)
    UINT VK;   // 键值
    UINT MOD;  // RegisterHotKey函数扩展键键值
    UINT HOT;  // HOTHKM_SETHOTKEY扩展键值
    //Mod->Hot
    UINT ModToHotkey(UINT fsModifiers, UINT VK) {
        if (VK == VK_UP || VK == VK_DOWN || VK == VK_LEFT || VK == VK_RIGHT) {
            fsModifiers |= HOTKEYF_EXT;//如果VK是扩展键(VK_UP,VK_DOWN,VK_LEFT,VK_RIGHT)则加上HOTKEYF_EXT
        }
        if ((fsModifiers & MOD_SHIFT) && !(fsModifiers & MOD_ALT)) // shift转alt
        {
            fsModifiers &= ~MOD_SHIFT;//如果fsModifiers有Shift则Mod->Hot
            fsModifiers |= HOTKEYF_SHIFT;
        }
        else if (!(fsModifiers & MOD_SHIFT) && (fsModifiers & MOD_ALT))// alt转shift
        {
            fsModifiers &= ~MOD_ALT;//如果fsModifiers有Alt则Mod->Hot
            fsModifiers |= HOTKEYF_ALT;
        }
        //Ctrl的键值相同不用改
        return fsModifiers;
    }
    //Hot->Mod
    UINT HotKeyToMod(UINT fsModifiers) {
        if ((fsModifiers & HOTKEYF_EXT)) {
            fsModifiers &= ~HOTKEYF_EXT;//如果fsModifiers有Shift则去掉Hot->Mod
        }
        if ((fsModifiers & HOTKEYF_SHIFT) && !(fsModifiers & HOTKEYF_ALT)) // shift转alt
        {
            fsModifiers &= ~HOTKEYF_SHIFT;//如果fsModifiers有Shift则Hot->Mod
            fsModifiers |= MOD_SHIFT;
        }
        else if (!(fsModifiers & HOTKEYF_SHIFT) && (fsModifiers & HOTKEYF_ALT)) // alt转shift
        {
            fsModifiers &= ~HOTKEYF_ALT;//如果fsModifiers有Alt则Mod->hot
            fsModifiers |= MOD_ALT;
        }
        //Ctrl的键值相同不用改
        return fsModifiers;
    }
    public:
    // 热键读入 
    friend BOOL HotkeyloadFlie();
    // 热键保存
    friend BOOL HotkeysaveFlie();
    // 注册热键
    friend BOOL HotkeyRegister(HWND);
    //构造函数
    HOTKEY() {
        SHIFT = CTRL = ALT = EXT = VK = MOD = HOT = 0;
    }
    // MOD值调用
    UINT ReleaseMOD() {
        return MOD;
    }
    // HOT值调用
    UINT ReleaseHOT() {
        return HOT;
    }
    // VK值调用
    UINT ReleaseVK() {
        return VK;
    }
    // 扩展键与键值改变
    void CreateAllKey(UINT fsModifiers, UINT vk, bool isMod) {
        VK = vk;
        //是Mod类热键
        if (isMod) {
            MOD = fsModifiers;
            HOT = ModToHotkey(fsModifiers, vk);
            if ((fsModifiers & MOD_SHIFT))SHIFT = 1;
            if ((fsModifiers & MOD_CONTROL))CTRL = 1;
            if ((fsModifiers & MOD_ALT))ALT = 1;
            if ((VK == VK_UP || VK == VK_DOWN || VK == VK_LEFT || VK == VK_RIGHT)) {
                EXT = 1;
            }
        }
        //是Hot类热键
        else {
            HOT = fsModifiers;
            MOD = HotKeyToMod(fsModifiers);
            if ((fsModifiers & HOTKEYF_SHIFT))SHIFT = 1;
            if ((fsModifiers & HOTKEYF_CONTROL))CTRL = 1;
            if ((fsModifiers & HOTKEYF_ALT))ALT = 1;
            if ((fsModifiers & HOTKEYF_EXT))EXT = 1;
        }
    }
}
hide,                                           // 隐藏窗口热键
output,                                         // 显示窗口热键
kill;                                           // 结束游戏热键

// 热键读入
BOOL HotkeyloadFlie() {
    fin.open(szCFGNAME[1]);
    if (!fin) {
        fin.close();
        if (ReleaseRes(szCFGNAME[1], IDR_CFG1, L"CFG")) {
            fin.open(szCFGNAME[1]);
        }
        else {
            MBBOXERR(L"释放资源文件2失败");
            PostQuitMessage(-1);
            return FALSE;
        }
    }
    fin >> hide.SHIFT >> hide.CTRL >> hide.ALT >> hide.EXT >> hide.VK >> hide.MOD >> hide.HOT;
    fin >> output.SHIFT >> output.CTRL >> output.ALT >> output.EXT >> output.VK >> output.MOD >> output.HOT;
    fin >> kill.SHIFT >> kill.CTRL >> kill.ALT >> kill.EXT >> kill.VK >> kill.MOD >> kill.HOT;
    fin.close();
    return TRUE;
}

// 热键保存
BOOL HotkeysaveFlie() {
    fout.open(szCFGNAME[1]);
    if (!fout) {
        MBBOXERR(L"保存热键失败");
        PostQuitMessage(-1);
        return FALSE;
    }
    fout << hide.SHIFT << " " << hide.CTRL << " " << hide.ALT << " " << hide.EXT << " " << hide.VK << " " << hide.MOD << " " << hide.HOT << endl;
    fout << output.SHIFT << " " << output.CTRL << " " << output.ALT << " " << output.EXT << " " << output.VK << " " << output.MOD << " " << output.HOT << endl;
    fout << kill.SHIFT << " " << kill.CTRL << " " << kill.ALT << " " << kill.EXT << " " << kill.VK << " " << kill.MOD << " " << kill.HOT << endl;
    fout.close();
    return TRUE;
}

// 释放资源
BOOL ReleaseRes(wstring strFileName, WORD wResID, wstring strFileType) {
    // 资源大小
    DWORD	dwWrite = 0;

    // 创建文件
    HANDLE	hFile = CreateFile(strFileName.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        MBBOXERR(L"CreateFile失败");
        return FALSE;
    }

    // 查找资源文件中、加载资源到内存、得到资源大小
    HRSRC	hrsc = ::FindResource(NULL, MAKEINTRESOURCE(wResID), strFileType.c_str());
    if (hrsc == NULL) {
        MBBOXERR(L"寻找资源失败");
        return FALSE;
    }
    else {
        HGLOBAL hG = LoadResource(NULL, hrsc);
        DWORD	dwSize = SizeofResource(NULL, hrsc);
        WriteFile(hFile, hG, dwSize, &dwWrite, NULL);
        CloseHandle(hFile);
        return TRUE;
    }
    // 写入文件
}

// 注册热键
BOOL HotkeyRegister(HWND hWnd) {
    init = 0;
    UnregisterHotKey(hWnd, IDK_HIDE);
    UnregisterHotKey(hWnd, IDK_OUTPUT);
    UnregisterHotKey(hWnd, IDK_KILL);
    DialogBox(hInst, MAKEINTRESOURCE(IDD_HOTKEYSET), hWnd, hotkeyset);
    if (!RegisterHotKey(hWnd, IDK_HIDE, hide.MOD, hide.VK)) {
        MBBOXATT(L"检测到热键冲突:隐藏窗口");
        return FALSE;
    }
    if (!RegisterHotKey(hWnd, IDK_OUTPUT, output.MOD, output.VK)) {
        MBBOXATT(L"检测到热键冲突:恢复窗口");
        return FALSE;
    }
    if (!RegisterHotKey(hWnd, IDK_KILL, kill.MOD, kill.VK)) {
        MBBOXATT(L"检测到热键冲突:恢复窗口");
        return FALSE;
    }
    return 1;
}

// 填充组合框
BOOL FillComboBox() {
    fin.open(szCFGNAME[0]);
    if (!fin) {
        fin.close();
        if (ReleaseRes(szCFGNAME[0], IDR_CFG2, L"CFG")) {
            fin.open(szCFGNAME[0]);
        }
        else {
            MBBOXERR(L"释放资源文件1失败");
            PostQuitMessage(-1);
            return FALSE;
        }
    }
    wstring now;
    fin >> Comboboxlong;
    getline(fin, now);
    getline(fin, NowChooce);
    for (int i = 1; i <= Comboboxlong; i++) {
        getline(fin, now);
        ComboBox_AddString(hCombobox, now.c_str());
    }
    ComboBox_SetCurSel(hCombobox, ComboBox_FindString(hCombobox, -1, NowChooce.c_str()));
    MoveWindow(hCombobox, 0, cychar * 3, 25 * cxchar, cychar * (3 + Comboboxlong), TRUE);
    fin.close();
    return TRUE;
}

// 计算时间
wstring WriteTimeString() {
    wstring Text;
    Text = L"已愉快";
    int sec, min, hour, day;
    int time = Time;
    sec = time % 60;
    time /= 60;
    min = time % 60;
    time /= 60;
    hour = time % 24;
    time /= 24;
    day = time;
    if (day > 0) {
        strio << day;
        Text += strio.str();
        Text += L"天";
        strio.str(L"");
        strio.clear();
    }
    if (hour > 0) {
        strio << hour;
        Text += strio.str();
        Text += L"小时";
        strio.str(L"");
        strio.clear();
    }
    if (min > 0) {
        strio << min;
        Text += strio.str();
        Text += L"分钟";
        strio.str(L"");
        strio.clear();
    }
    if (sec >= 0) {
        strio << sec;
        Text += strio.str();
        Text += L"秒";
        strio.str(L"");
        strio.clear();
    }
    return Text;
}

BOOL CompareVersion(wstring Version1,wstring Version2) {
    return 1;
}
// 主函数
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance,
                      _In_ LPWSTR    lpCmdLine,
                      _In_ int       nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此处放置代码。
    result = FMOD::System_Create(&System);
    if (result != FMOD_OK) {
        MBBOXERR(L"FMOD声音系统创建失败");
        return FALSE;
    }
    result = System->init(512, FMOD_INIT_NORMAL, 0);
    if (result != FMOD_OK) {
        MBBOXERR(L"FMOD声音系统初始化失败");
        return FALSE;
    }
    result = System->createChannelGroup("001", &channelgroup);
    if (result != FMOD_OK) {
        MBBOXERR(L"FMOD声音通道组创建失败");
        return FALSE;
    }

    // 流对象语言初始化
    loc = fin.imbue(locale("chs"));
    loc = fout.imbue(locale("chs"));
    loc = strio.imbue(locale("chs"));

    // 屏幕宽高获取
    cxscreen = GetSystemMetrics(SM_CXFULLSCREEN);
    cyscreen = GetSystemMetrics(SM_CYFULLSCREEN);

    // 默认字符宽高获取
    cxchar = LOWORD(GetDialogBaseUnits());
    cychar = HIWORD(GetDialogBaseUnits());

    // 初始化全局字符串
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_MINECRAFTCONTROLOR, szWindowClass, MAX_LOADSTRING);
    LoadString(hInstance, IDS_CFGNAME1, szCFGNAME[0], MAX_LOADSTRING);
    LoadString(hInstance, IDS_CFGNAME2, szCFGNAME[1], MAX_LOADSTRING);

    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance(hInstance, nCmdShow)) {
        MBBOXERR(L"应用程序初始化失败");
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MINECRAFTCONTROLOR));

    MSG msg;

    // 主消息循环:
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance) {
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MINECRAFTCONTROLOR));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCE(IDC_MINECRAFTCONTROLOR);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
    return RegisterClassEx(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口(并加互斥锁)
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
    hInst = hInstance; // 将实例句柄存储在全局变量中
    hMutex = CreateMutex(NULL, FALSE, L"Minecraft Controlor App");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        CloseHandle(hMutex);
        MBBOXATT(L"你已启动");
        return FALSE;
    }
    HWND hWnd = CreateWindow(
        szWindowClass,
        szTitle,
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
        CW_USEDEFAULT,
        0,
        CW_USEDEFAULT,
        0,
        nullptr,
        nullptr,
        hInstance,
        nullptr);

    if (!hWnd) {
        MBBOXERR(L"窗口创建失败");
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}



//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE:
        {
            //读入热键
            HotkeyloadFlie();
            //设置计时器
            SetTimer(hWnd, IDT_TIMER, 1000, NULL);
            //保存窗口句柄
            hProcess = hWnd;
            //保存菜单句柄
            hMenu = GetMenu(hWnd);
            //初始化注册热键
            if (!RegisterHotKey(hWnd, IDK_HIDE, hide.ReleaseMOD(), hide.ReleaseVK())) {
                MBBOXATT(L"检测到热键冲突:隐藏窗口");
                while (!HotkeyRegister(hWnd));
            }
            if (!RegisterHotKey(hWnd, IDK_OUTPUT, output.ReleaseMOD(), output.ReleaseVK())) {
                MBBOXATT(L"检测到热键冲突:恢复窗口");
                while (!HotkeyRegister(hWnd));
            }
            if (!RegisterHotKey(hWnd, IDK_KILL, kill.ReleaseMOD(), kill.ReleaseVK())) {
                MBBOXATT(L"检测到热键冲突:恢复窗口");
                while (!HotkeyRegister(hWnd));
            }
            //窗口居中
            MoveWindow(hWnd, (cxscreen - wide) / 2, (cyscreen - high) / 2, wide, high, TRUE);
            //创建按钮
            hButton = CreateWindow(
                WC_BUTTON,
                L"删除该项",
                WS_CHILDREN | BS_PUSHBUTTON,
                0,
                cychar * 5,
                10 * cxchar,
                7 * cychar / 4,
                hWnd,
                (HMENU)IDB_DELETE,
                (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE),
                NULL);
            //创建组合框
            hCombobox = CreateWindow(
                WC_COMBOBOX,
                NULL,
                WS_CHILDREN | CBS_DROPDOWNLIST,
                0,
                cychar * 3,
                25 * cxchar,
                cychar * (3 + Comboboxlong),
                hWnd,
                (HMENU)IDCB_COMBOBOX,
                (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE),
                NULL);
            //填充
            FillComboBox();
            //播放音乐
            srand((unsigned int)time(NULL));
            music = IDR_BGM1 + rand() % 26;
            ReleaseRes(L"001.music", music, L"BGM");
            result = System->createSound("001.music", FMOD_DEFAULT, 0, &Sound);
            if (result != FMOD_OK) {
                MBBOXERR(L"Fmod创建声音失败");
                PostQuitMessage(-1);
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
            result = System->playSound(Sound, channelgroup, 0, &channel);
            if (result != FMOD_OK) {
                MBBOXERR(L"Fmod播放声音失败");
                PostQuitMessage(-1);
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
            result = channel->setVolume((float)0.1);
            if (result != FMOD_OK) {
                MBBOXERR(L"Fmod 设置声量失败");
                PostQuitMessage(-1);
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;
        }
        case WM_COMMAND:
        {
            int LoWord = LOWORD(wParam);
            int HiWord = HIWORD(wParam);
            // 分析菜单选择:
            if (lParam == 0) {
                switch (LoWord) {
                    //暂停音乐
                    case IDM_PAUSEMUSIC:
                    {
                        if (isPause) {
                            ModifyMenu(hMenu, IDM_PAUSEMUSIC, MF_STRING | MF_BYCOMMAND, IDM_PAUSEMUSIC, L"暂停(&P)");
                        }
                        else {
                            ModifyMenu(hMenu, IDM_PAUSEMUSIC, MF_STRING | MF_BYCOMMAND, IDM_PAUSEMUSIC, L"播放(&P)");
                        }
                        isPause = !isPause;
                        result = channel->setPaused(isPause);
                        if (result != FMOD_OK) {
                            MBBOXERR(L"fmod音乐菜单暂停or播放设置失败");
                            PostQuitMessage(-1);
                            return DefWindowProc(hWnd, message, wParam, lParam);
                        }
                        break;
                    }
                    //关于
                    case IDM_ABOUT:
                    {
                        DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, DialogDefault);
                        break;
                    }
                    //下一首
                    case IDM_NEXTMUSIC:
                    {
                        isPause = 0;
                        Sound->release();
                        music++;
                        if (music > IDR_BGM26) {
                            music = IDR_BGM1;
                        }
                        ReleaseRes(L"001.music", music, L"BGM");
                        result = System->createSound("001.music", FMOD_DEFAULT, 0, &Sound);
                        if (result != FMOD_OK) {
                            MBBOXERR(L"Fmod创建声音失败");
                            PostQuitMessage(-1);
                            return DefWindowProc(hWnd, message, wParam, lParam);
                        }
                        result = System->playSound(Sound, channelgroup, 0, &channel);
                        if (result != FMOD_OK) {
                            MBBOXERR(L"Fmod播放声音失败");
                            PostQuitMessage(-1);
                            return DefWindowProc(hWnd, message, wParam, lParam);
                        }
                        result = channel->setVolume((float)0.1);
                        if (result != FMOD_OK) {
                            MBBOXERR(L"Fmod 设置声量失败");
                            PostQuitMessage(-1);
                            return DefWindowProc(hWnd, message, wParam, lParam);
                        }
                        break;
                    }
                    //上一首
                    case IDM_UPMUSIC:
                    {
                        isPause = 0;
                        Sound->release();
                        music--;
                        if (music < IDR_BGM1) {
                            music = IDR_BGM26;
                        }
                        ReleaseRes(L"001.music", music, L"BGM");
                        result = System->createSound("001.music", FMOD_DEFAULT, 0, &Sound);
                        if (result != FMOD_OK) {
                            MBBOXERR(L"Fmod创建声音失败");
                            PostQuitMessage(-1);
                            return DefWindowProc(hWnd, message, wParam, lParam);
                        }
                        result = System->playSound(Sound, channelgroup, 0, &channel);
                        if (result != FMOD_OK) {
                            MBBOXERR(L"Fmod播放声音失败");
                            PostQuitMessage(-1);
                            return DefWindowProc(hWnd, message, wParam, lParam);
                        }
                        result = channel->setVolume((float)0.1);
                        if (result != FMOD_OK) {
                            MBBOXERR(L"Fmod 设置声量失败");
                            PostQuitMessage(-1);
                            return DefWindowProc(hWnd, message, wParam, lParam);
                        }
                        break;
                    }
                    //说明
                    case IDM_INSTRUCTION:
                    {
                        DialogBox(hInst, MAKEINTRESOURCE(IDD_INSTRUCTION), hWnd, DialogDefault);
                        break;
                    }
                    //退出
                    case IDM_EXIT:
                    {
                        DestroyWindow(hWnd);
                        break;
                    }
                    //设置热键
                    case IDM_HOTKEYSET:
                    {
                        while (!HotkeyRegister(hWnd));
                        break;
                    }
                    //重置时间
                    case IDM_RESETTIME:
                    {
                        Time = 0;
                        MBBOXATT(L"时间已重置");
                        break;
                    }
                    case IDM_SETVOLUME:
                    {
                        init = 0;
                        DialogBox(hInst, MAKEINTRESOURCE(IDD_SETVOLUME), hWnd, SetVolume);
                        break;
                    }
                    //其他
                    default:
                    {
                        return DefWindowProc(hWnd, message, wParam, lParam);
                    }
                }
            }
            //其他
            else {
                switch (HiWord) {
                    /*用户改变了组合框的选择*/
                    case CBN_SELCHANGE:
                    {
                        WCHAR A[MAX_LOADSTRING];
                        ComboBox_GetText(hCombobox, A, MAX_LOADSTRING);
                        if (wcscmp(A, L"") == 0) {
                            DialogBox(hInst, MAKEINTRESOURCE(IDD_INSERT), hWnd, Insert);
                        }
                        else {
                            NowChooce = A;
                        }
                        MoveWindow(hCombobox, 0, cychar * 3, 25 * cxchar, cychar * (3 + Comboboxlong), TRUE);
                        InvalidateRect(hWnd, NULL, FALSE);
                        break;
                    }
                    /*用户展开了组合框*/
                    case CBN_DROPDOWN:
                    {
                        ComboBox_AddString(hCombobox, L"not found?");
                        MoveWindow(hCombobox, 0, cychar * 3, 25 * cxchar, cychar * (3 + Comboboxlong + 1), TRUE);
                        break;
                    }
                    /*用户按下按钮*/
                    case BN_CLICKED:{
                        SendMessage(hButton, BM_SETSTATE, 1, 0);
                        if (Comboboxlong > 1) {
                            int chooce = ComboBox_GetCurSel(hCombobox);
                            ComboBox_DeleteString(hCombobox, ComboBox_GetCurSel(hCombobox));
                            if (chooce == 0) {
                                ComboBox_SetCurSel(hCombobox, 0);
                            }
                            else if (chooce == Comboboxlong - 1) {
                                ComboBox_SetCurSel(hCombobox, chooce - 1);
                            }
                            else {
                                ComboBox_SetCurSel(hCombobox, chooce);
                            }
                            Comboboxlong--;
                        }
                        WCHAR A[MAX_LOADSTRING];
                        ComboBox_GetText(hCombobox, A, MAX_LOADSTRING);
                        NowChooce = A;
                        SendMessage(hButton, BM_SETSTATE, 0, 0);
                        break;
                    }
                    case CBN_CLOSEUP:
                    {
                        ComboBox_DeleteString(hCombobox, Comboboxlong);
                        MoveWindow(hCombobox, 0, cychar * 3, 25 * cxchar, cychar* (3 + Comboboxlong), TRUE);
                        break;
                    }
                }
            }
            break;
        }
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            RECT rect;
            wstring Text = L"暂停中!";
            if (isPause == 0) {
                Text = L"播放中!";
            }
            GetClientRect(hWnd, &rect);
            if (!ishide)Text += WriteTimeString();
            Text += L"                                                      \n";
            Text += L"当前版本:Minecraft ";
            Text += NowChooce;
            Text += L"                                                      \n";
            if (hMinecraft == NULL) {
                Text += L"游戏未启动                                        ";
            }
            else {
                Text += L"游戏已启动                                         ";
            }
            // TODO: 在此处添加使用 hdc 的任何绘图代码...
            DrawText(hdc, Text.c_str(), -1, &rect, 0);
            EndPaint(hWnd, &ps);
            break;
        }
        case WM_DESTROY:
        {
            //释放声音
            Sound->release();
            DeleteFile(L"001.music");
            System->release();
            channelgroup->release();
            //停止计时
            KillTimer(hWnd, IDT_TIMER);
            //保存版本
            WCHAR A[MAX_LOADSTRING];
            fout.open(szCFGNAME[1]);
            int x = Comboboxlong;
            fout << x << endl;
            fout << NowChooce << endl;
            for (int i = 1; i <= x; i++) {
                memset(A, 0, sizeof(A));
                ComboBox_GetLBText(hCombobox, i - 1, A);
                fout << A << endl;
            }
            fout.close();
            //保存热键
            HotkeysaveFlie();
            //退出
            PostQuitMessage(0);
            break;
        }
        case WM_TIMER:
        {
            wstring game = L"Minecraft ";
            game += NowChooce;
            hMinecraft = FindWindow(NULL, game.c_str());
            if (!ishide && hMinecraft != NULL)Time++;
            bool p = 0;
            bool ex = 0;
            channel->getPaused(&p);
            channel->isPlaying(&ex);
            SetFocus(hWnd);
            InvalidateRect(hWnd, NULL, FALSE);
            break;
        }
        case WM_HOTKEY:
        {
            switch (wParam) {
                case IDK_HIDE:
                    ishide = 1;
                    isPause = 1;
                    ModifyMenu(hMenu, IDM_PAUSEMUSIC, MF_STRING | MF_BYCOMMAND, IDM_PAUSEMUSIC, L"播放(&P)");
                    result = channel->setPaused(isPause);
                    if (result != FMOD_OK) {
                        MBBOXERR(L"fmod音乐暂停设置失败");
                        PostQuitMessage(-1);
                        return DefWindowProc(hWnd, message, wParam, lParam);
                    }
                    ShowWindow(hMinecraft, 0);
                    ShowWindow(hWnd, 0);
                    break;
                case IDK_OUTPUT:
                    ishide = 0;
                    ShowWindow(hWnd, 1);
                    ShowWindow(hMinecraft, 1);
                    break;
                case IDK_KILL:
                    SendMessage(hMinecraft, WM_CLOSE, 0, 0);
                    break;
            }

        }
        default:
        {
            return DefWindowProc(hWnd, message, wParam, lParam);
        }    
    }
    return 0;
}

// 对话框的消息默认处理。
INT_PTR CALLBACK DialogDefault(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    UNREFERENCED_PARAMETER(lParam);
    switch (message) {
        case WM_INITDIALOG:
            return (INT_PTR)TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
            break;
    }
    return (INT_PTR)FALSE;
}

//热键设置对话框消息处理
INT_PTR CALLBACK hotkeyset(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    //初始化热键窗口
    if (init == 1) {
        SendDlgItemMessage(hDlg, IDC_HIDE, HKM_SETHOTKEY, MAKEWORD((UINT)hide.ReleaseVK(), (UINT)hide.ReleaseHOT()), 0);
        SendDlgItemMessage(hDlg, IDC_OUTPUT, HKM_SETHOTKEY, MAKEWORD((UINT)output.ReleaseVK(), (UINT)output.ReleaseHOT()), 0);
        SendDlgItemMessage(hDlg, IDC_KILL, HKM_SETHOTKEY, MAKEWORD((UINT)kill.ReleaseVK(), (UINT)kill.ReleaseHOT()), 0);
        init = 2;
    }
    else {
        if (init != 2) {
            init = 1;
        }
    }

    switch (message) {
        case WM_CREATE:
        {
            break;
        }
        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK) {
                hide.CreateAllKey(HIBYTE(LOWORD(SendDlgItemMessage(hDlg, IDC_HIDE, HKM_GETHOTKEY, 0, 0))), LOBYTE(LOWORD(SendDlgItemMessage(hDlg, IDC_HIDE, HKM_GETHOTKEY, 0, 0))), 0);
                output.CreateAllKey(HIBYTE(LOWORD(SendDlgItemMessage(hDlg, IDC_OUTPUT, HKM_GETHOTKEY, 0, 0))), LOBYTE(LOWORD(SendDlgItemMessage(hDlg, IDC_OUTPUT, HKM_GETHOTKEY, 0, 0))), 0);
                kill.CreateAllKey(HIBYTE(LOWORD(SendDlgItemMessage(hDlg, IDC_KILL, HKM_GETHOTKEY, 0, 0))), LOBYTE(LOWORD(SendDlgItemMessage(hDlg, IDC_KILL, HKM_GETHOTKEY, 0, 0))), 0);
                EndDialog(hDlg, TRUE);
            }
            if (LOWORD(wParam) == IDCANCEL) {
                EndDialog(hDlg, TRUE);
            }
            break;
    }
    return FALSE;
}

// 插入新选择的对话框消息处理程序
INT_PTR CALLBACK Insert(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    UNREFERENCED_PARAMETER(lParam);
    WCHAR insert[26] = {0};
    int numofch = 0;
    switch (message) {
        case WM_INITDIALOG:
            // Set the default push button to "Cancel." 
            SendMessage(hDlg, DM_SETDEFID, (WPARAM)IDCANCEL, (LPARAM)0);

            return TRUE;

        case WM_COMMAND:
            // Set the default push button to "OK" when the user enters text. 
            if (HIWORD(wParam) == EN_CHANGE &&
                LOWORD(wParam) == IDC_EDIT1) {
                SendMessage(hDlg,
                            DM_SETDEFID,
                            (WPARAM)IDOK,
                            (LPARAM)0);
            }
            switch (wParam) {
                case IDOK:
                    // Get number of characters. 
                    numofch = (WORD)SendDlgItemMessage(hDlg, IDC_EDIT1, EM_LINELENGTH, (WPARAM)0, (LPARAM)0);
                    if (numofch == 0) {
                        MessageBox(hDlg,
                                   L"你啥也没有输入",
                                   L"提示",
                                   MB_OK | MB_DEFBUTTON1);
                        ComboBox_SetCurSel(hCombobox, ComboBox_FindString(hCombobox, -1, NowChooce.c_str()));
                        EndDialog(hDlg, TRUE);
                        return FALSE;
                    }
                    if (numofch >= 25) {
                        MessageBox(hDlg,
                                   L"太多了,你是不是想骗我",
                                   L"提示",
                                   MB_OK | MB_DEFBUTTON1);
                        ComboBox_SetCurSel(hCombobox, ComboBox_FindString(hCombobox, -1, NowChooce.c_str()));
                        EndDialog(hDlg, TRUE);
                        return FALSE;
                    }
                    // Put the number of characters into first word of buffer. 
                    *((LPWORD)insert) = numofch;
                    // Get the characters. 
                    SendDlgItemMessage(hDlg, IDC_EDIT1, EM_GETLINE, (WPARAM)0, (LPARAM)insert);
                    insert[numofch] = 0;
                    if (ComboBox_FindString(hCombobox, -1, insert) == CB_ERR) {
                        NowChooce = insert;
                        ComboBox_AddString(hCombobox, insert);
                        Comboboxlong++;
                        ComboBox_SetCurSel(hCombobox, Comboboxlong - 1);
                    }
                    else {
                        MessageBox(hDlg,
                                   L"你已经输入过这个版本",
                                   L"提示",
                                   MB_OK | MB_DEFBUTTON1);
                        ComboBox_SetCurSel(hCombobox, ComboBox_FindString(hCombobox, -1, NowChooce.c_str()));
                    }
                    EndDialog(hDlg, TRUE);
                    return TRUE;
                case IDCANCEL:
                    ComboBox_SetCurSel(hCombobox, ComboBox_FindString(hCombobox, -1, NowChooce.c_str()));
                    EndDialog(hDlg, TRUE);
                    return TRUE;
            }
            return 0;
    }

    UNREFERENCED_PARAMETER(lParam);
    return (INT_PTR)FALSE;
}

// 声音设置对话框消息处理
INT_PTR CALLBACK SetVolume(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    //初始化声音设置窗口
    if (init == 1) {
        SendDlgItemMessage(hDlg, IDC_SLIDER1, TBM_SETRANGEMAX, TRUE, 100);
        SendDlgItemMessage(hDlg, IDC_SLIDER1, TBM_SETRANGEMIN, TRUE, 0);
        SendDlgItemMessage(hDlg, IDC_SLIDER1, TBM_SETPOS, TRUE, 90);
        init = 2;
    }
    else {
        if (init != 2) {
            init = 1;
        }
    }
    int LoWord = LOWORD(wParam);
    int Pos;
    switch (message) {
        case WM_VSCROLL:
        {
            switch (LoWord) {
                case TB_THUMBTRACK:
                {
                    Pos = SendDlgItemMessage(hDlg, IDC_SLIDER1, TBM_GETPOS, 0, 0);
                    result=channel->setVolume(float((float)(100 - Pos)) / ((float)100));
                    if (result != FMOD_OK) {
                        MBBOXERR(L"Fmod 设置声量失败");
                        PostQuitMessage(-1);
                        EndDialog(hDlg, LOWORD(wParam));
                    }
                    break;
                }
            }
        }
        case WM_COMMAND:
        {
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
            break;
        }
        default:
            break;
    }
    return (INT_PTR)FALSE;
}
#include "dialog.h"
#include "ui_dialog.h"
#include <QDebug>

#include <windows.h>
#pragma comment(lib, "user32.lib")

#include <WtsApi32.h>
#pragma comment(lib, "Wtsapi32.lib")

// 自定義事件編號
#define WM_POWER_STATUS_MSG    (WM_USER + 1001)

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
    WTSRegisterSessionNotification((HWND)this->winId(), NOTIFY_FOR_THIS_SESSION);

    // 變更Window Title
    QString strWindowTitle = "NativeEventFilter Dialog";
    setWindowTitle(strWindowTitle);
    QString strGetWindowTitle;
    strGetWindowTitle = windowTitle();
    qDebug() <<"strGetWindowTitle:" << strGetWindowTitle;

    string strTitle;
    strTitle = q2s(strGetWindowTitle);
    qDebug() << "strTitle.c_str():" << strTitle.c_str();
    qDebug() << "strTitle.data():" << strTitle.data();

    getWindowTitle();
    QStringformat();
}

Dialog::~Dialog()
{
    delete ui;
    WTSUnRegisterSessionNotification((HWND)this->winId());
}

bool Dialog::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
#if defined(Q_WS_WIN) || defined(Q_OS_WIN)
    // Windows OS
    if (eventType == "windows_generic_MSG" || eventType == "windows_dispatcher_MSG")
    {
        MSG *pMsg = reinterpret_cast<MSG *>(message);
        //MSG *pMsg = static_cast<MSG *>(message);

        // 視窗的非客戶區域
        if (pMsg->message == WM_NCMOUSEMOVE)
        {
            // 獲取系統滑鼠游標移動
            qDebug() << "(WM_NCMOUSEMOVE) nativeEventFilter:" << "(" << pMsg->pt.x << ","   << pMsg->pt.y << ")";
        }

        // 視窗的客戶區域
        if (pMsg->message == WM_MOUSEMOVE)
        {
            // 獲取系統滑鼠游標移動
            qDebug() << "(WM_MOUSEMOVE) nativeEventFilter:" << "(" << pMsg->pt.x << ","   << pMsg->pt.y << ")";
        }

        if (pMsg->message == WM_POWERBROADCAST)
        {
             switch(pMsg->wParam)
             {
                case PBT_APMPOWERSTATUSCHANGE:
                    qDebug() << "PBT_APMPOWERSTATUSCHANGE received\n";
                    break;

                case PBT_APMRESUMEAUTOMATIC:
                    qDebug() << "PBT_APMRESUMEAUTOMATIC received\n";
                    // 通過主窗口類名尋找主窗口句炳
                    SendMessageW((HWND)this->winId(), WM_POWER_STATUS_MSG, 0, 0);

                    // 使用PostMessage需要接到事件後回覆已處理完畢
                    //PostMessageW((HWND)this->winId(), WM_POWER_STATUS_MSG, 0, 0);
                    break;

                case PBT_APMRESUMESUSPEND:
                    qDebug() << "PBT_APMRESUMESUSPEND received\n";
                    break;

                case PBT_APMSUSPEND:
                    qDebug() << "PBT_APMSUSPEND received\n";
                    break;

                case PBT_APMQUERYSUSPEND:
                    qDebug() << "PBT_APMQUERYSUSPEND received\n";
                    break;

                case PBT_POWERSETTINGCHANGE:
                    qDebug() << "PBT_POWERSETTINGCHANGE received\n";
                    break;

                default:
                    qDebug() << "Not Define\n";
                    break;
             }
         }

         // 自定義事件編號
         if(pMsg->message == WM_POWER_STATUS_MSG)
         {
             qDebug() << "WM_POWER_STATUS_MSG received\n";
             CheckBatteryStatus();

             // 使用PostMessage需要接到事件後回覆已處理完畢，不再繼續傳遞
             //return true;
         }

         // 關機、關閉程式或視窗
         if(pMsg->message == WM_ENDSESSION)
         {
            qDebug() << "WM_ENDSESSION received\n";
         }

         // 關機、重啟、登出
         if(pMsg->message == WM_QUERYENDSESSION)
         {
            qDebug() << "WM_QUERYENDSESSION received\n";
         }

         if (pMsg->message == WM_WTSSESSION_CHANGE)
         {
             qDebug() << "pMsg->wParam =" << pMsg->wParam;
             switch (pMsg->wParam)
             {
                case WTS_CONSOLE_CONNECT:
                    qDebug() << "WTS_CONSOLE_CONNECT received\n";
                    break;

                 case WTS_CONSOLE_DISCONNECT:
                     qDebug() << "WTS_CONSOLE_DISCONNECT received\n";
                     break;

                 case WTS_REMOTE_CONNECT:
                     qDebug() << "WTS_REMOTE_CONNECT received\n";
                     break;

                 case WTS_REMOTE_DISCONNECT:
                     qDebug() << "WTS_REMOTE_DISCONNECT received\n";
                     break;

                 case WTS_SESSION_LOGON:
                     qDebug() << "WTS_SESSION_LOGON received\n";
                     break;

                 case WTS_SESSION_LOGOFF:
                     qDebug() << "WTS_SESSION_LOGOFF received\n";
                     break;

                 case WTS_SESSION_LOCK:
                     qDebug() << "WTS_SESSION_LOCK received\n";
                     break;

                 case WTS_SESSION_UNLOCK:
                     qDebug() << "WTS_SESSION_UNLOCK received\n";
                     break;

                 case WTS_SESSION_REMOTE_CONTROL:
                     qDebug() << "WTS_SESSION_REMOTE_CONTROL received\n";
                     break;

                 case WTS_SESSION_CREATE:
                     qDebug() << "WTS_SESSION_CREATE received\n";
                     break;

                 case WTS_SESSION_TERMINATE:
                     qDebug() << "WTS_SESSION_TERMINATE received\n";
                     break;

                 default:
                     qDebug() << "Not Define\n";
                     break;
             }
             return true;
         }
    }
#elif defined(Q_WS_X11) || defined(Q_OS_LINUX)
    // Linux OS
    if (eventType == "xcb_generic_event_t")
    {
        xcb_generic_event_t *ev = static_cast <xcb_generic_event_t *> (message);
        // Do Something here ...
    }
#endif
    return false;
}

void Dialog::CheckBatteryStatus()
{
    SYSTEM_POWER_STATUS ps;
    GetSystemPowerStatus(&ps);

    int nLife = 0;
    // 是否使用交流電源
    if ( ps.ACLineStatus != AC_LINE_ONLINE )
    {
       if ( ps.BatteryFlag != BATTERY_FLAG_NO_BATTERY && ps.BatteryFlag != BATTERY_FLAG_UNKNOWN )
       {
            nLife = ps.BatteryLifePercent;
            if ( nLife <= 10 )
            {
               // 電源低於10%
                qDebug() << "The Power Less 10%";
            }
            else
            {
                qDebug() << "The Power Than 10%" ;
            }
        }
     }
     else
     {
        // 有電源供電
        qDebug() << "Have Power Supply";
     }
}

void Dialog::getWindowTitle()
{
// 在Windows裡，為了同時適用於ASCII字串與Unicode字串的處理 (以是否有定義"UNICODE"做為區別)，特別定義了TCHAR型態，在有定義UNICODE時視同WCHAR，否則便是char
#ifdef  _UNICODE
    qDebug() << "is WCHAR";
#else
    qDebug() << "is char";
#endif

    TCHAR titleText[MAX_PATH] = {0};
    HWND hWnd = (HWND)this->winId();
    GetWindowText(hWnd, titleText, MAX_PATH);

    // 獲取字串長度
    int iLength = WideCharToMultiByte(CP_ACP, 0, titleText, -1, NULL, 0, NULL, NULL);

    // 將tchar值賦給char
    char ansiStr[MAX_PATH] = {0};
    WideCharToMultiByte(CP_ACP, 0, titleText, -1, ansiStr, iLength, NULL, NULL);
    qDebug() << "ansiStr:" <<  ansiStr;

    // 將tchar值賦給char
    // 動態配置記憶體記得釋放記憶體
    //char* ansiStr = new char[iLength+1];
    //ansiStr[0] = '\0';

    //WideCharToMultiByte(CP_ACP, 0, titleText, -1, ansiStr, iLength, NULL, NULL);
    //qDebug() << "ansiStr:" <<  ansiStr;

    // 釋放動態記憶體
    //if(ansiStr)
    //  delete [] ansiStr;
}

// 將 string 轉換成 QString
QString Dialog::s2q(const string &s)
{
    return QString(QString::fromLocal8Bit(s.c_str()));
}

// 將 QString 轉換成 string
string Dialog::q2s(const QString &s)
{
    return string((const char *)s.toLocal8Bit());
}

void Dialog::QStringformat()
{
    // Qt中函數替換MFC中CString.format()函數
    char buf[128] = { 0 };
    ::snprintf(buf, 128, "%lld", 123456789LL);
    QString str1 = QString::fromLatin1(buf);
    qDebug() << "QStringformat" << "str1:" <<  str1;

    QString str2 = QString("%1").arg(123456789LL);
    qDebug() << "QStringformat" << "str2:" <<  str2;

    char tmp[128] = { 0 };
    //sprintf(tmp, "-w \"%s\"", "Dialog");
    sprintf(tmp, "%s", "-w \"Dialog\"");
    QString str3 = QString::fromLatin1(tmp);
    qDebug() << "QStringformat" << "str3:" <<  str3;

    string strStr3;
    strStr3 = q2s(str3);
    qDebug() << "strStr3.c_str():" << strStr3.c_str();

    char tmpbuf[128] = { 0 };
    ::snprintf(tmpbuf, 128, "%s", "-w \"Dialog\"");
    QString str4 = QString::fromLatin1(tmpbuf);
    qDebug() << "QStringformat" << "str4:" <<  str4;
    string strStr4;
    strStr4 = q2s(str4);
    qDebug() << "strStr4.c_str():" << strStr4.c_str();

    QString str5 = QString("%1").arg("-w \"Dialog\"");
    qDebug() << "QStringformat" << "str5:" << str5;
    string strStr5;
    strStr5 = q2s(str5);
    qDebug() << "strStr5.c_str():" << strStr5.c_str();
}

void Dialog::on_pushButton_clicked()
{
    QString strGetWindowTitle;
    strGetWindowTitle = windowTitle();
    qDebug() <<"strGetWindowTitle:" << strGetWindowTitle;

    string strTitle;
    strTitle = q2s(strGetWindowTitle);
    qDebug() << "strTitle.c_str():" << strTitle.c_str();
    qDebug() << "strTitle.data():" << strTitle.data();

    QString str5 = QString("%1").arg("-w \"Dialog\"");
    qDebug() << "QStringformat" << "str5:" << str5;
    string strStr5;
    strStr5 = q2s(str5);
    qDebug() << "strStr5.c_str():" << strStr5.c_str();

    const size_t len = strStr5.length() + 1;
    wchar_t *wcstring = new wchar_t[len];

    //string to wchar_t *
    swprintf(wcstring, len, L"%S", strStr5.c_str());

    //wchar_t * to QString
    QString qStrout = QString::fromWCharArray(wcstring);
    qDebug() << "wcstring" << qStrout;

    ShellExecute(NULL, L"open", L"notepad.exe", NULL, wcstring, SW_SHOW);
    if(wcstring)
        delete [] wcstring;
    qDebug() << GetLastError();

    ShellExecute(NULL, L"open", L"calc.exe", NULL, NULL, SW_SHOWNORMAL);
    qDebug() << GetLastError();
}

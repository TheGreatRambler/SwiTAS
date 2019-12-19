#pragma once

#include <switch.h>

// Many thanks to 黯然的饭#8969 for this implementation
class GetScreenshot {
private:
    u64 VI_pid;
    // The Title Id of VI (The display manager)
    constexpr u64 VITitleId = 0x010000000000002D;
    Handle VIdbg;
    // Size of the framebuffer
    // Width * Height * Byte depth
    // 1280 * 720 * 4
    constexpr u64 framebufferSize = 3686400;

public:
    // The pointer to the shared framebuffer
    // Data is swizzled https://github.com/averne/dvdnx/blob/master/src/screen.hpp#L74
    u8* framebuffer;
    
    GetScreenshot() {
        // Get the PID from the Title ID
        pmdmntGetProcessId(&VI_pid, VITitleId);
        // Create the framebuffer
        framebuffer = new u8[framebufferSize];
    }
    
    void get() {
        // Get the framebuffer as of now
        u64 rc = svcDebugActiveProcess(&VIdbg, VI_pid);
        if(R_SUCCEEDED(rc))
        {
            u64 addr = 0;
            MemoryInfo VI_Info;
            for(int i = 0; i < 0x1000; i++)
            {
                MemoryInfo info;
                u32 pageinfo;
                rc = svcQueryDebugProcessMemory(&info, &pageinfo, VIdbg, addr);
                if(info.type == MemType_CodeMutable)
                {
                    if(info.size == 0x1A40000)
                    {
                        VI_Info = info;
                        break;
                    }
                }
                else if(info.type == MemType_Reserved)
                {
                    break;
                }
                addr += info.size;
            }
            svcReadDebugProcessMemory(framebuffer, VIdbg, VI_Info.addr, framebufferSize);
            svcCloseHandle(VIdbg);
        }
    }
    
    ~GetScreenshot() {
        // Delete the buffer now that it is not needed
        delete framebuffer[];
    }
}

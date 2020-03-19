#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <cstdarg>
#include <string>
#include <cstdlib>
#include <switch.h>

#include "script_init.hpp"

std::string keyDef[] = {"KEY_A", "KEY_B", "KEY_X", "KEY_Y", "KEY_LSTICK", "KEY_RSTICK", "KEY_L", "KEY_R", "KEY_ZL", "KEY_ZR", "KEY_PLUS", "KEY_MINUS", "KEY_DLEFT", "KEY_DUP", "KEY_DRIGHT", "KEY_DDOWN"};

u64 translateKey(std::string str)
{  
    for (u64 i = 0; i < sizeof(keyDef) / sizeof(char *); i++)
    {
        if (str == keyDef[i])
        {
            return BIT(i);
        }
    }
    return static_cast<u64>(0);
}

void log_to_sd_out(const char *fmt, ...) 
{
    FILE* f = fopen("/output.log", "a");
    va_list myargs;
    va_start(myargs, fmt);
    vfprintf(f, fmt, myargs);
    va_end(myargs);
    fclose(f);
}

std::vector<struct controlMsg> getScriptLines(std::string fileName)
{
    std::vector<struct controlMsg> script;

    std::ifstream ifs;
    ifs.open(fileName.c_str(), std::fstream::in);

    //log_to_sd_out(fileName.c_str());
    //log_to_sd_out("\n");

    if(ifs.is_open())
    {
        std::string keyStr, lStickStr, rStickStr;
        int frame;
        while(ifs >> frame >> keyStr >> lStickStr >> rStickStr)
        {
            struct controlMsg tempMsg = lineAsControlMsg(frame, keyStr, lStickStr, rStickStr);

            script.push_back(tempMsg);
        }
    }

    return script;
}

struct controlMsg lineAsControlMsg(int frame, std::string keyStr, std::string lStickStr, std::string rStickStr)
{
    //get keys
    u64 keys = 0;
    while(keyStr.find_last_of(";") != std::string::npos)
    {
        std::size_t found = keyStr.find_last_of(";");
        std::string latestKey = keyStr.substr(found+1);
        keys |= translateKey(latestKey);
        keyStr.resize(found);
    }
    keys |= translateKey(keyStr);

    //get left stick
    std::size_t foundL = lStickStr.find(";");
    std::string l_x_pos = lStickStr.substr(0,foundL);
    s32 joy_l_x = static_cast<s32>(std::stoi(l_x_pos));
    std::string l_y_pos = lStickStr.substr(foundL+1);
    s32 joy_l_y = static_cast<s32>(std::stoi(l_y_pos));

    //get right stick
    std::size_t foundR = rStickStr.find(";");
    std::string r_x_pos = rStickStr.substr(0,foundR);
    s32 joy_r_x = static_cast<s32>(std::stoi(r_x_pos));
    std::string r_y_pos = rStickStr.substr(foundR+1);
    s32 joy_r_y = static_cast<s32>(std::stoi(r_y_pos));


    //deposit found values into the script
    struct controlMsg message;
    message.frame = frame;
    message.keys = keys;
    message.joy_l_x = joy_l_x;
    message.joy_l_y = joy_l_y;
    message.joy_r_x = joy_r_x;
    message.joy_r_y = joy_r_y;

    return message;
}

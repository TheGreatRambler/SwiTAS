#include <cstring>
#include <cstdlib>
#include "script_init.hpp"

#include "script_provider.hpp"

std::shared_ptr<struct controlMsg> ScriptProvider::nextLine()
{
    return pullFromQueue();
}
bool ScriptProvider::hasNextLine()
{
    return !queueIsEmpty();
}
void ScriptProvider::populateQueue()
{}

bool LineStreamScriptProvider::hasNextLine()
{
    return !(stream.eof() && queueIsEmpty());
}
void LineStreamScriptProvider::populateQueue()
{
    if(shouldPopulate())
    {
        int frame;
        std::string keyStr, lStickStr, rStickStr;
        while(queueSize() < 30 && !stream.eof())
        {
            stream >> frame >> keyStr >> lStickStr >> rStickStr;
            struct controlMsg msg = lineAsControlMsg(frame, keyStr, lStickStr, rStickStr);
            std::shared_ptr<struct controlMsg> sharedPtr = std::make_shared<struct controlMsg>(msg);
            pushToQueue(sharedPtr);
        }
        if(stream.eof())
        {
            afterEOF();
        }
    }
}

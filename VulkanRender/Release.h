#pragma once

struct Release
{
    virtual void release() = 0;
    
protected:
    ~Release() = default;
};
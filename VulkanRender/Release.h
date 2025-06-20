#pragma once

struct Release
{
    virtual void release() const = 0;
protected:
    ~Release() = default;
};
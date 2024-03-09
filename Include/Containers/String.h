#pragma once

namespace Ember
{
    namespace CStringUtil
    {
        static size_t StringLength(const char* String)
        {
            const char* Temp;
            for(Temp = String; *Temp; ++Temp) {}
            return (Temp - String);
        }
    }
    struct String
    {
    public:
        String() : CString(nullptr), Length(0) {}
        String(const char* Input) : CString(Input), Length(CStringUtil::StringLength(Input)) {}
        size_t GetLength() const { return Length; }
        const char* CStr() const { return CString; }
        
    private:
        const char* CString;
        size_t Length;
    };
}

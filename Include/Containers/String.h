#pragma once

namespace Ember
{
    namespace CStringUtil
    {
        static size_t StringLength(const char* String)
        {
            const char* Temp = String;
            while(*Temp)
            {
                ++Temp;
            }
            return (Temp - String);
        }

        static char* StringCopy(const char* Source)
        {
            size_t Length = StringLength(Source) + 1;
            char* Destination = new char[Length];
            for(size_t Index = 0; Index < Length; ++Index)
            {
                Destination[Index] = Source[Index];
            }

            return Destination;
        }

        static size_t StringCompare(const char* String1, const char* String2)
        {
            while(*String1 && (*String1 == *String2))
            {
                ++String1;
                ++String2;
            }
            
            return(*String1 - *String2);
        }
    }
    
    struct String
    {
    public:
        String() : CString(nullptr), Length(0) {}
        String(const char* Input)
        {
            if(!Input)
            {
                CString = nullptr;
                Length = 0;
                return;
            }

            Length = CStringUtil::StringLength(Input);
            CString = CStringUtil::StringCopy(Input);
        }

        String(const String& Other)
        {
            Length = Other.Length;
            CString = CStringUtil::StringCopy(Other.CString);
        }

        ~String()
        {
            delete[] CString;
            Length = 0;
        }

        String& operator=(const String& Other)
        {
            if(this != &Other)
            {
                delete[] CString;
                Length = Other.Length;
                CString = CStringUtil::StringCopy(Other.CString);
            }

            return *this;
        }
        
        size_t GetLength() const { return Length; }
        const char* CStr() const { return CString; }
        
    private:
        char* CString;
        size_t Length;
    };
}

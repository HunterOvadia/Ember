#pragma once
static size_t CStringLength(const char* String)
{
    const char* Temp = String;
    while(*Temp)
    {
        ++Temp;
    }
    return (Temp - String);
}

static char* CStringCopy(const char* Source)
{
    size_t Length = CStringLength(Source) + 1;
    char* Destination = EmberMemoryAllocateType<char>(Length);
    for(size_t Index = 0; Index < Length; ++Index)
    {
        Destination[Index] = Source[Index];
    }

    return Destination;
}

inline size_t CStringCompare(const char* String1, const char* String2)
{
    while(*String1 && (*String1 == *String2))
    {
        ++String1;
        ++String2;
    }
    
    return(*String1 - *String2);
}


struct string_t
{
public:
    string_t() : CString(nullptr), Length(0) {}
    string_t(const char* Input)
    {
        if(!Input)
        {
            CString = nullptr;
            Length = 0;
            return;
        }

        Length = CStringLength(Input);
        CString = CStringCopy(Input);
    }

    string_t(const string_t& Other)
    {
        Length = Other.Length;
        CString = CStringCopy(Other.CString);
    }

    ~string_t()
    {
        delete[] CString;
        Length = 0;
    }

    string_t& operator=(const string_t& Other)
    {
        if(this != &Other)
        {
            EmberMemoryFree(CString);
            Length = Other.Length;
            CString = CStringCopy(Other.CString);
        }

        return *this;
    }
    
    size_t GetLength() const { return Length; }
    const char* CStr() const { return CString; }
    
private:
    char* CString;
    size_t Length;
};

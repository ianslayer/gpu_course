
inline bool CharIsPrintable(int c)
{
    return ( c >= 0x20 && c <= 0x7E ) || ( c >= 0xA1 && c <= 0xFF );
}

inline bool CharIsLower(int c)
{
    // test for regular ascii and western European high-ascii chars
    return ( c >= 'a' && c <= 'z' ) || ( c >= 0xE0 && c <= 0xFF );
}

inline bool CharIsUpper(int c)
{
    // test for regular ascii and western European high-ascii chars
    return ( c <= 'Z' && c >= 'A' ) || ( c >= 0xC0 && c <= 0xDF );
}

inline bool CharIsAlpha(int c)
{
    // test for regular ascii and western European high-ascii chars
    return ( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) ||
        ( c >= 0xC0 && c <= 0xFF ) );
}

inline bool CharIsNumeric(int c)
{
    return ( c <= '9' && c >= '0' );
}

inline bool CharIsNewLine(char c)
{
    return ( c == '\n' || c == '\r' || c == '\v' );
}

inline bool CharIsTab(char c)
{
    return ( c == '\t' );
}

inline bool CharIsBlank(char c)
{
    if(c == 32|| c ==' ' || c== '\t' || c== '\n' || c== '\r')
        return true;
    return false;
}
#include "CQEnum_Pch.hpp"
#include "CQEnum.hpp"

namespace CQSL { namespace CQEnum {

//
//  Gets the next token and makes sure it's the indicated value. Can either throw or
//  return false if not found. If it indicates there must be an equal sign, then we
//  always throw if we find the token but not the equals.
//
bool InputSrc::bCheckNextId(const   std::string_view&   svCheck
                            , const char* const         pszFailMsg
                            , const bool                bWithEquals
                            , const bool                bThrowIfIfNot)
{
    if ((eGetNextToken(m_strTmpToken) != CQEnum::ETokens::Identifier)
    ||  (m_strTmpToken != svCheck))
    {
        if (bThrowIfIfNot)
        {
            ThrowParseErr(pszFailMsg);
        }

        // Push the characters back
        PushBack(m_strTmpToken);
        return false;
    }

    if (bWithEquals)
    {
        if (eGetNextToken(m_strTmpToken) != CQEnum::ETokens::Equals)
        {
            ThrowParseErr("Expected an equal sign here");
        }
    }
    return true;
}


//
//  When running through a parent block we are always lookging for either
//  a child block or the end of the parent block. This returns true if a
//  child block or false if the end of the parent. Throws if not one of those.
//
//  We assume the child block also includes an equal sign.
//
bool InputSrc::bNextChildOrEnd( const   char* const pszBlock
                                , const char* const pszEndBlock)
{

    // The next token must be an id
    if (eGetNextToken(m_strTmpToken) != CQEnum::ETokens::Identifier)
    {
        ThrowParseErr("Expected an id here");
    }

    // If it's the child block, check the equal sign
    bool bRes = false;
    if (m_strTmpToken == pszBlock)
    {
        // We have to have an equal sign
        if (eGetNextToken(m_strTmpToken) != CQEnum::ETokens::Equals)
        {
            ThrowParseErr("Expected an equal sign following start of block");
        }
        bRes = true;
    }
    else if (m_strTmpToken == pszEndBlock)
    {
        // Just fall through with false return
    }
     else
    {
        std::string strErrMsg("Expected a '");
        strErrMsg.append(pszBlock);
        strErrMsg.append("' child block or the end of the '");
        strErrMsg.append(pszEndBlock);
        strErrMsg.append("' parent block");
        ThrowParseErr(strErrMsg);
    }
    return bRes;
}



// Checks for the end line for a block, throws if not
void InputSrc::CheckBlockEnd(const char* const pszCheck)
{
    m_strTmpTest = "End";
    m_strTmpTest.append(pszCheck);
    bCheckNextId(m_strTmpTest, "Expected end of current block", false, true);
}


// Makes sure the next token is a colon
void InputSrc::CheckColon()
{
    if (eGetNextToken(m_strTmpToken) != CQEnum::ETokens::Colon)
    {
        ThrowParseErr("Expected a colon here");
    }
}


// Makes sure the next token is a comma
void InputSrc::CheckComma()
{
    if (eGetNextToken(m_strTmpToken) != CQEnum::ETokens::Comma)
    {
        ThrowParseErr("Expected a comma here");
    }
}


// Makes sure the next token is an equal sign
void InputSrc::CheckEqualSign()
{
    if (eGetNextToken(m_strTmpToken) != CQEnum::ETokens::Equals)
    {
        ThrowParseErr("Expected a equals sign here");
    }
}


//
//  Makes sure the next token seen is an (optionally signed) numeric value. If not
//  it throw.
//
int32_t InputSrc::iCheckSignedValue(const   char* const     pszCheck
                                    , const char* const     pszFailMsg)
{
    // Has to start wtih xxx=
    bCheckNextId(pszCheck, pszFailMsg, true, true);

    // And next we should get a number token
    if (eGetNextToken(m_strTmpToken) != CQEnum::ETokens::Number)
    {
        ThrowParseErr("Expected a signed number here");
    }

    // Convert first to 64 bit so we can test limits
    size_t szEnd = 0;
    int64_t iRet = std::stoll(m_strTmpToken, &szEnd, 16);
    if (szEnd != m_strTmpToken.length())
    {
        ThrowParseErr(pszFailMsg);
    }
    
    if ((iRet < std::numeric_limits<int32_t>::min())
    ||  (iRet > std::numeric_limits<int32_t>::max()))
    {
        ThrowParseErr("Expected a number in the int32_t range");
    }
    return static_cast<int32_t>(iRet);
}


// Gets the next token and insures it's a numeric value, throws if not
int32_t InputSrc::iGetSignedToken(const char* const pszFailMsg)
{
    // And next we should get a number token
    if (eGetNextToken(m_strTmpToken) != CQEnum::ETokens::Number)
    {
        ThrowParseErr("Expected a signed number here");
    }

    // Figure out radix
    uint32_t radix = 10;
    if (m_strTmpToken.find("0x", 0) != std::string::npos)
    {
        m_strTmpToken.erase(0, 2);
        radix = 16;
    }

    // Convert first to 64 bit so we can test limits
    size_t szEnd = 0;
    int64_t iRet = std::stoll(m_strTmpToken, &szEnd, radix);
    if (szEnd != m_strTmpToken.length())
    {
        ThrowParseErr(pszFailMsg);
    }
    
    if ((iRet < std::numeric_limits<int32_t>::min())
    ||  (iRet > std::numeric_limits<int32_t>::max()))
    {
        ThrowParseErr(pszFailMsg);
    }
    return static_cast<int32_t>(iRet);
}


// Reads the rest of the current line
void InputSrc::GetLineRemainder(std::string& strToFill)
{
    strToFill.clear();

    // Rmemeber this as the position of this 'token'
    m_uLastTokenCol = m_uColNum;
    m_uLastTokenLine = m_uLineNum;

    char chCur = chGetNext(false);
    while (chCur != 0xA)
    {
        strToFill.push_back(chCur);
        chCur = chGetNext(false);
    }
}


// Get the next token and validates that it's an id type
void InputSrc::GetIdToken(const char* const pszFailMsg, std::string& strToFill)
{
    if (eGetNextToken(strToFill) != ETokens::Identifier)
    {
        ThrowParseErr(pszFailMsg);
    }
}


// Reads the rest of the line as a set of space separated tokens
void InputSrc::GetSpacedValues(std::vector<std::string>& vToFill)
{
    vToFill.clear();

    // Rmemeber this as the position of this 'token'
    m_uLastTokenCol = m_uColNum;
    m_uLastTokenLine = m_uLineNum;

    m_strTmpToken.clear();
    bool bInToken = false;
    while (true)
    {
        const char chCur = chGetNext(false);
        const bool bIsWS = bIsSpace(chCur);
        
        if (bInToken)
        {
            // If we hit a space, then the end, else accumulate
            if (bIsWS)
            {
                if (!m_strTmpToken.empty())
                {
                    vToFill.push_back(m_strTmpToken);
                }
                m_strTmpToken.clear();
                bInToken = false;
            }
            else
            {
                m_strTmpToken.push_back(chCur);
            }
        }
        else
        {
            // Ignore spaces, wait for start of a new token
            if (!bIsWS)
            {
                m_strTmpToken.clear();
                m_strTmpToken.push_back(chCur);
                bInToken = true;
            }
        }
        
        // If we hit the end of the line, we are done
        if (chCur == 0xA)
        {
            break;
        }
    }
}


// Reads the rest of the line as a set of comma separated tokens, no leading/trailing space
void InputSrc::GetCommaSepValues(std::vector<std::string>& vToFill)
{
    vToFill.clear();

    // Rmemeber this as the position of this 'token'
    m_uLastTokenCol = m_uColNum;
    m_uLastTokenLine = m_uLineNum;

    m_strTmpToken.clear();
    bool bInToken = false;
    while (true)
    {
        const char chCur = chGetNext(false);
        const bool bIsWS = bIsSpace(chCur);
        
        if (bInToken)
        {
            // If we hit a comma, then the end, else accumulate
            if (chCur == ',')
            {
                // Get rid of any trailing white space
                while (!m_strTmpToken.empty() && std::isspace(m_strTmpToken.back()))
                {
                    m_strTmpToken.pop_back();
                }

                if (!m_strTmpToken.empty())
                {
                    vToFill.push_back(m_strTmpToken);
                }
                m_strTmpToken.clear();
                bInToken = false;
            }
            else
            {
                m_strTmpToken.push_back(chCur);
            }
        }
        else
        {
            // Ignore spaces, wait for start of a new token
            if (!bIsWS)
            {
                // If it's a comma, then we had an empty token
                if (chCur == ',')
                {
                    m_strTmpToken.clear();
                    vToFill.push_back(m_strTmpToken);
                }
                else
                {
                    m_strTmpToken.clear();
                    m_strTmpToken.push_back(chCur);
                    bInToken = true;
                }
            }
        }
        
        //
        //  If we hit the end of the line, we are done, but we have to
        //  deal with a trailing token.
        //
        if (chCur == 0xA)
        {
            if (bInToken)
            {
                // Get rid of any trailing white space
                while (!m_strTmpToken.empty() && std::isspace(m_strTmpToken.back()))
                {
                    m_strTmpToken.pop_back();
                }

                if (!m_strTmpToken.empty())
                {
                    vToFill.push_back(m_strTmpToken);
                }
            }
            break;
        }
    }
}

// Get the next token and validates that it's a quoted string type
std::string InputSrc::strGetToken(const char* const pszFailMsg)
{
    std::string strRet;
    if (eGetNextToken(strRet) != ETokens::String)
    {
        ThrowParseErr(pszFailMsg);
    }
    return strRet;
}


// Get the next lexical token from the input stream
ETokens InputSrc::eGetNextToken(std::string& strText)
{
    strText.clear();

    // The first character tells us what we are looking at, ignore space
    char chCur = chGetNext(true);
    if (!chCur)
        return ETokens::End;

    //
    //  Remember this as the last token position. Sub one from the column because
    //  we just ate one above. This cannot underflow since we have to have just
    //  eaten at least one character on the current line.
    //
    m_uLastTokenCol = m_uColNum - 1;
    m_uLastTokenLine = m_uLineNum;

    ETokens eRet = ETokens::Identifier;
    if (chCur == ',')
    {
        eRet = ETokens::Comma;
    }
    else if (chCur == ':')
    {
        eRet = ETokens::Colon;
    }
    else if (chCur == '=')
    {
        eRet = ETokens::Equals;
    }
    else if (chCur == '"')
    {
        eRet = ETokens::String;

        // Gather up until the closing quote
        while (m_strmSrc)
        {
            // Can't ignore white space in this case
            chCur = chGetNext(false);
            if (chCur == 0)
            {
                // Illegal end of file
                ThrowParseErr("Illegal end of file inside quoted string");
            }
            else if (chCur == 0x0A)
            {
                ThrowParseErr("String literal crossed a line boundary");
            }

            // Break out if the end, else add to the string
            if (chCur == '"')
                break;
            strText.push_back(chCur);
        }
    }
    else if (bIsDigit(chCur, false) || (chCur == '+') || (chCur == '-'))
    {
        eRet = ETokens::Number;

        // Eat characters until we get an end separator character
        strText.push_back(chCur);
        while (m_strmSrc)
        {
            chCur = chGetNext(false);
            if (bIsEndSep(chCur))
            {
                // If not space, push it back
                if (!bIsSpace(chCur))
                {
                    PushBack(chCur);
                }
                break;
            }
            strText.push_back(chCur);
        }

        //
        //  Try to convert the value to a number. If it starts with 0x,
        //  then strip that off and parse as a hex number
        //
        uint32_t radix = 10;
        if ((strText.length() >= 2) && (std::strncmp(strText.c_str(), "0x", 2) == 0))
        {
            strText.erase(0, 2);
            radix = 16;
        }

        char* pszEnd;
        int64_t iVal = std::strtol(strText.c_str(), &pszEnd, radix);

        // If the end isn't pointing at a null, then it's bad
        if (*pszEnd)
        {
            ThrowParseErr("Could not convert to a number");
        }

        // If hex, put the hex prefix back
        if (radix == 16)
        {
            strText.insert(0, "0x");
        }
    }
    else if (bIsIdChar(chCur, true))
    {
        eRet = ETokens::Identifier;

        // It's a token, so we read until we hit an end sep char
        strText.push_back(chCur);
        while (m_strmSrc)
        {
            // Can't ignore white space in this case
            chCur = chGetNext(false);
            if (bIsEndSep(chCur))
            {
                // If not space, push it back
                if (!bIsSpace(chCur))
                {
                    PushBack(chCur);
                }
                break;
            }
            strText.push_back(chCur);
        }
    }
    else
    {
        ThrowParseErr("Expected a valid id, number or special character");
    }
    return eRet;
}


bool InputSrc::bIsDigit(const char chTest, const bool bHex) const noexcept
{
    if (bHex)
        return std::isxdigit(static_cast<unsigned char>(chTest));

    return std::isdigit(static_cast<unsigned char>(chTest));
}


bool InputSrc::bIsIdChar(const char chTest, const bool first) const noexcept
{
    const bool bIsAlpha = std::isalpha(chTest);

    // If the first char it has to be alpha
    if (first)
        return bIsAlpha;
    
    // Else it can be alphanum, underscore or hyphen
    return bIsAlpha || bIsDigit(chTest, true) || (chTest == '_') || (chTest == '-');
}


//
//  We can legally end ids and numbers and such either on white space or one
//  of the separator characters. It can be an LF character if the thing being 
//  consumed is at the end of the line, but that will count as space so we get
//  that naturally.
//
bool InputSrc::bIsEndSep(const char chTest) const noexcept
{
    return 
    (
        (chTest == ')')
        || (chTest == ',')
        || (chTest == '=')
        || (chTest == ':')
        || bIsSpace(chTest) 
    );
}


bool InputSrc::bIsSpace(const char chTest) const noexcept
{
    return std::isspace(static_cast<unsigned char>(chTest));
}


// Builds up an error message that includes the last token position info
std::string InputSrc::BuildErrMsg(const std::string_view& svMsg) const
{
    std::string strFullMsg = "(Line=";
    strFullMsg.append(std::to_string(m_uLastTokenLine));
    strFullMsg.append("/Col=");
    strFullMsg.append(std::to_string(m_uLastTokenCol));
    strFullMsg.append(") - ");
    strFullMsg.append(svMsg);
 
    return strFullMsg;
}


// Return the next character, optionally ignoring white space
char InputSrc::chGetNext(const bool ignoreWS)
{
    char chRet = 0;
    while (m_strmSrc)
    {
        m_strmSrc.get(chRet);

        if (!bIsSpace(chRet))
        {
            m_uColNum++;

            //
            //  If this is the first non-space of this line and it's a semi-colon,
            //  it's a comment so we need to eat the rest of the line and then
            //  continue.
            //
            if (m_bFirstLineChar && (chRet == ';'))
            {
                while (m_strmSrc)
                {
                    m_strmSrc.get(chRet);
                    if ((chRet == 0x0A) || (chRet == 0x0D))
                    {
                        // This character will be processed below
                        break;
                    }
                }

                if (!m_strmSrc)
                {
                    // Return a zero to indicate end of input
                    chRet = 0;
                    break;
                }
            }
            else
            {
                // Nothing special, so break out with this character
                m_bFirstLineChar = false;
                break;
            }
        }

        // Handle newlines specially
        if ((chRet == 0x0D) || (chRet == 0x0A))
        {
            // If a CR, see if there's a subsequent LF. If not put it back
            if (chRet == 0xD)
            {
                char chTmp;
                m_strmSrc.get(chTmp);
                if (chTmp != 0x0A)
                    m_strmSrc.putback(chTmp);
            }

            m_bFirstLineChar = true;
            m_uLineNum++;
            m_uColNum = 1;
        }
         else
        {
            m_uColNum++;
        }

        // If not ignoring white space, break out
        if (!ignoreWS)
        {
            break;
        }
    }
    return chRet;
}


//
//  The main program calls this to get us to open the source stream and prepare
//  to parse.
//
void InputSrc::Open(const std::string& strSrcFile)
{
    m_strmSrc.open(strSrcFile, std::ifstream::in);
    if (!m_strmSrc)
    {
        throw std::runtime_error("The input file could not be opened");
    }

    m_bFirstLineChar = true;
    m_uColNum = 1;
    m_uLineNum = 1;
    m_uLastTokenCol = 1;
    m_uLastTokenLine = 1;
}


// Push back a single character to the stream
void InputSrc::PushBack(const char chToPush)
{
    m_strmSrc.putback(chToPush);

    // We should never push back over a new line
    if (!m_uColNum)
    {
        ThrowParseErr("Pushback column underflow error!");
    }
    m_uColNum--;
}


// Push back a string to the stream
void InputSrc::PushBack(const std::string& strToPush)
{
    std::string::const_reverse_iterator rit = strToPush.rbegin();
    while (rit != strToPush.rend())
    {
        m_strmSrc.putback(*rit);

        // We should never push back over a new line
        if (!m_uColNum)
        {
            ThrowParseErr("Pushback column underflow error!");
        }
        ++rit;
    }
}

 
}};

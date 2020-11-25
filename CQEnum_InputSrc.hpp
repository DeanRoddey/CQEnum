#pragma once

//
//  This file defines the input source we use for parsing the .cqenum
//  file. It's the usual thing, in that it wraps an input stream, 
//  tracks current location (for error output), and provides lexical
//  type parsing to break out tokens based on the known separator
//  characters and such.
//
//  On top of that we provide helpers to extract particular token types
//  or check that the next token meets some criteria. 
//

namespace CQSL { namespace CQEnum {

// The categories of tokens we understand
enum class ETokens
{
    End
    , Colon
    , Comma
    , Equals
    , Identifier
    , Number
    , String
};


class InputSrc
{
    public :
        InputSrc() = default;
        ~InputSrc() = default;

        // Unimplemented
        InputSrc(const InputSrc&) = delete;
        InputSrc(InputSrc&&) = delete;
        InputSrc& operator=(const InputSrc&) = delete;
        InputSrc& operator=(InputSrc&&) = delete;

        //
        //  Check if the next token is an id type with the indicated name,
        //  optionally checking if it is followed by an equals sign. This
        //  saves a lot of busy work.
        //
        bool bCheckNextId
        (
            const   char* const         pszCheck
            , const char* const         pszFailMsg
            , const bool                bWithEquals
            , const bool                bThrowIfNot
        );

        //
        //  Used within blocks to look for the next child or the end of block,
        //  which is done in many of the parse's loops.
        //
        bool bNextChildOrEnd
        (
            const   char* const         pszBlock
            , const char* const         pszEndBlock
        );

        // The next input must be the end of the block of the given name
        void CheckBlockEnd
        (
            const   char* const         pszCheck
        );

        void CheckColon();

        void CheckComma();

        void CheckEqualSign();

        //
        //  The next input  must be a key=value line where the value has 
        //  the indicated name and the value is a signed number.
        //
        int32_t iCheckSignedValue
        (
            const   char* const         pszCheck
            , const char* const         pszFailMsg
        );

        // The next input must be a signed number
        int32_t iGetSignedToken
        (
            const   char* const         pszFailMsg
        );
        
        // Get the next lexical token from the input stream
        ETokens eGetNextToken(std::string& strText);

        // Return the remainder of the current line
        void GetLineRemainder
        (
                    std::string&        strToFill
        );

        // The next input must be an id token, returns the id text
        void GetIdToken
        (
            const   char* const         pszFailMsg
            ,       std::string&        strToFill
        );

        //
        //  The next input must be a list of comma separated values
        //  which are returned as separated values.
        //
        void GetCommaSepValues
        (
                    std::vector<std::string>& vToFill
        );

        //
        //  The next input must be a list of space separated values
        //  which are returned as separated values.
        //
        void GetSpacedValues
        (
                    std::vector<std::string>& vToFill
        );

        // The next input must be a quoted string, returns the text
        std::string strGetToken
        (
            const   char* const         pszFailMsg
        );

        void Open(const std::string& strSrcFile);

        //
        //  The throw needs to be visible to the code analyzer, so we do it
        //  this way.
        //
        void ThrowParseErr(const char* const pszMsg)
        {
            throw std::runtime_error(BuildErrMsg(pszMsg));
        }

        void ThrowParseErr(const std::string& strMsg)
        {
            ThrowParseErr(strMsg.c_str());
        }


    private :
        // Is the char either a hex or decimal digit
        bool bIsDigit(const char chTest, const bool bHex) const noexcept;

        //
        //  Is the character valid for an identifier. First char is more restricted
        //  that subsequent ones.
        //
        bool bIsIdChar(const char chTest, const bool bFirst) const noexcept;

        //
        //  Is the character a valid end of token separator, either white space or
        //  special characters that always separate tokens (if not currently within
        //  a quoted value.)
        //
        bool bIsEndSep(const char chTest) const noexcept;

        bool bIsSpace(const char chTest) const noexcept;

        std::string BuildErrMsg(const char* const pszErr) const;

        // Return the next character, optionally ignoring white space
        char chGetNext(const bool ignoreWS);

        void PushBack(const char chToPush);

        void PushBack(const std::string& strToPush);

        bool            m_bFirstLineChar = true;

        // The current line and column
        uint32_t        m_uColNum = 0;
        uint32_t        m_uLineNum = 0;

        //
        //  The line/col of the start of the last parsed token, or the 
        //  one currently being parsed. This is what we use for error
        //  messages.
        //
        uint32_t        m_uLastTokenCol = 0;
        uint32_t        m_uLastTokenLine = 0;

        std::ifstream   m_strmSrc;

        //
        //  Some temp strings for internal use. Don't assume they are unaffected
        //  across any call to any other method here.
        //
        std::string     m_strTmpToken;
        std::string     m_strTmpTest;
};

}};


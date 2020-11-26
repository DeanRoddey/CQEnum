#include "CQEnum_Pch.hpp"
#include "CQEnum.hpp"


namespace CQSL { namespace CQEnum {


//
//  At this point the Val= part has been eaten. So we should see the value
//  name, and then any of the attributes of the value. We get the flags that
//  tell us which of them must be present.
//
void EnumValInfo::ParseFrom(        InputSrc&       srcFile
                            , const bool            bOrdinal
                            , const bool            bAltVal
                            , const bool            bText1
                            , const bool            bText2
                            , const std::string&    strEnumName)
{
    // We have to get the name of the enum
    srcFile.GetIdToken("Expected enumeration value name", m_strName);

    const std::string strEndBlock("EndVal");
    std::string strCurToken;
    std::string strCurValue;
    bool        bGotAltVal = false;
    bool        bGotOrdinal = false;
    bool        bGotText1 = false;
    bool        bGotText2 = false;
    while (true)
    {
        srcFile.GetIdToken("Expected enum attribute or end of value block", strCurToken);

        // If the end of the enum block, then we are done
        if (strCurToken == strEndBlock)
        {
            break;
        }

        // We should have an equal sign
        srcFile.CheckEqualSign();

        if ((strCurToken == "Text1")
        ||  (strCurToken == "Text2"))
        {
            // We expect a quoted string here
            strCurValue = srcFile.strGetToken("Expected a quoted text value");

            if (strCurToken == "Text1")
            {
                bGotText1 = true;
                m_strText1 = std::move(strCurValue);
            }
            else if (strCurToken == "Text2")
            {
                bGotText2 = true;
                m_strText2 = std::move(strCurValue);
            }
        }
        else if (strCurToken == "Ordinal")
        {
            bGotOrdinal = true;
            m_iOrdinal = srcFile.iGetSignedToken("Expected a numeric ordinal value");
        }
        else if (strCurToken == "AltVal")
        {
            bGotAltVal = true;
            m_iAltValue = srcFile.iGetSignedToken("Expected a numeric alternative value");
        }
    }

    // Make sure we got what we we supposed to and nothing we weren't
    if (bGotAltVal != bAltVal)
    {
        std::string strErrMsg
        (
            bGotAltVal ? "Alt values are not enabled for enum " : "Missing alt value for enum "
        );
        strErrMsg.append(strEnumName);
        throw std::runtime_error(strErrMsg);
    }

    if (bGotOrdinal != bOrdinal)
    {
        std::string strErrMsg
        (
            bGotOrdinal ? "Arbitrary ordinals are not enabled for enum "
                        : "Missing ordinal value for enum "
        );
        strErrMsg.append(strEnumName);
        throw std::runtime_error(strErrMsg);
    }

    if (bGotText1 != bText1)
    {
        std::string strErrMsg
        (
            bGotText1 ? "Text value 1 is not enabled for enum " : "Missing text value 1 for enum "
        );
        strErrMsg.append(strEnumName);
        throw std::runtime_error(strErrMsg);
    }

    if (bGotText2 != bText2)
    {
        std::string strErrMsg
        (
            bGotText2 ? "Text value 2 is not enabled for enum " : "Missing text value 2 for enum "
        );
        strErrMsg.append(strEnumName);
        throw std::runtime_error(strErrMsg);
    }
}


//
//  At this point, the Enum= of the block start line has been eaten. So we
//  get the enum name, then go into the block and parse out all the info and
//  values.
//
void EnumInfo::ParseFrom(InputSrc& srcFile)
{
    // Make sure we are at defaults
    m_bIsMonotonic = false;
    m_bText1 = false;
    m_bText2 = false;
    m_bDoAltVal = false;
    m_bDoInc = false;
    m_eType = EEnumTypes::Count;
    m_strName.clear();
    m_strUnderType = "int";
    m_vSynonyms.clear();
    m_vValues.clear();

    // We have to get a name next
    srcFile.GetIdToken("Expected to see enumeration name", m_strName);

    //
    //  Now we can have a set of key=value. Some of them are just attributes 
    //  of the enum, some will be the opening of Val= blocks. We have to see
    //  attributes before any values.
    //
    const std::string strEndBlock("EndEnum");
    std::string strCurToken;
    std::string strCurValue;
    EnumValInfo evaliNew;
    int64_t iHighestVal = 0;
    std::vector<std::string> vFlags;
    while (true)
    {
        srcFile.GetIdToken("Expected attribute, enum value block, or synonym", strCurToken);

        // If the end of the enum block, then we are done
        if (strCurToken == strEndBlock)
        {
            break;
        }

        // Else has to be an attribute, synonym, or enum value
        if ((strCurToken == "Type")
        ||  (strCurToken == "UnderType")
        ||  (strCurToken == "Flags"))
        {
            // These have to come before any values
            if (!m_vValues.empty())
            {
                srcFile.ThrowParseErr("Enumeration attributes must be set before defining values");
            }

            srcFile.CheckEqualSign();

            if (strCurToken == "Type")
            {
                if (m_eType != EEnumTypes::Count)
                {
                    srcFile.ThrowParseErr("Enumeration type has already been set");
                }

                srcFile.GetIdToken("Expected type to follow equal sign", strCurValue);
                if (strCurValue == "Arbitrary")
                {
                    m_eType = EEnumTypes::Arbitary;
                    m_bIsMonotonic = true;
                }
                else if (strCurValue == "Bitmap")
                {
                    m_eType = EEnumTypes::Bitmap;
                }
                else if (strCurValue == "Standard")
                {
                    m_eType = EEnumTypes::Standard;
                    m_bIsMonotonic = true;
                }
                else
                {
                    std::string strErrMsg("'");
                    strErrMsg.append(strCurToken);
                    strErrMsg.append("' is not a valid enumeration type (Arbitrary, Bitmap, or Standard)");
                    srcFile.ThrowParseErr(strErrMsg);
                }
            }
            else if (strCurToken == "UnderType")
            {
                //
                //  This can be multiple values, so we just get the fest of the line. We don't pass
                //  any judgement on it.
                //
                srcFile.GetLineRemainder(m_strUnderType);
            }
            else if (strCurToken == "Flags")
            {
                srcFile.GetSpacedValues(vFlags);
                for (std::string strFlag : vFlags)
                {
                    if (strFlag == "AltVal")
                    {
                        m_bDoAltVal = true;
                    }
                    else if (strFlag == "Inc")
                    {
                        m_bDoInc = true;
                    }
                    else if (strFlag == "Text1")
                    {
                        m_bText1 = true;
                    }
                    else if (strFlag == "Text2")
                    {
                        m_bText2 = true;
                    }
                    else
                    {
                        std::string strErrMsg("'");
                        strErrMsg.append(strFlag);
                        strErrMsg.append("' is not a valid enumeration flag value (AltVal, Inc, Text1, Text2");
                        srcFile.ThrowParseErr(strErrMsg);
                    }
                }
            }
        }
        else if (strCurToken == "Synonym")
        {
            srcFile.CheckEqualSign();

            // We should get a 'name:enumval" pair, start with the name
            srcFile.GetIdToken("Expected synonym name after equal sign", strCurToken);

            // It can't have the same name as any existing synonym or value
            if ((itFindValue(strCurToken) != m_vValues.end())
            ||  (itFindSynonym(strCurToken) != m_vSynonyms.end()))
            {
                std::string strErrMsg("Name '");
                strErrMsg.append(strCurToken);
                strErrMsg.append("' is is already in use in enum '");
                strErrMsg.append(m_strName);
                strErrMsg.append("'");
                srcFile.ThrowParseErr(strErrMsg);                
            }

            srcFile.CheckColon();

            //
            //  We have either one or multiple comma separated values. Bitmapped enums
            //  can indicate multiple values and the synonym is an OR of those values.
            //
            if (m_eType == EEnumTypes::Bitmap)
            {
                std::vector<std::string> vSynValues;
                unsigned int uBits = 0;
                srcFile.GetCommaSepValues(vSynValues);
                for (std::string& strCur : vSynValues)
                {
                    std::vector<EnumValInfo>::iterator itVal = itFindValue(strCur);
                    if (itVal == m_vValues.end())
                    {
                        std::string strErrMsg("'");
                        strErrMsg.append(strCur);
                        strErrMsg.append("' does not refer to an existing value of enum '");
                        strErrMsg.append(m_strName);
                        strErrMsg.append("'");
                        srcFile.ThrowParseErr(strErrMsg);
                    }

                    uBits |= static_cast<unsigned int>(itVal->m_iOrdinal);
                }

                m_vSynonyms.emplace_back(strCurToken, uBits);
            }
            else
            {
                srcFile.GetIdToken("Expected synonym mapping value", strCurValue);

                std::vector<EnumValInfo>::iterator itVal = itFindValue(strCurValue);
                if (itVal == m_vValues.end())
                {
                    std::string strErrMsg("'");
                    strErrMsg.append(strCurValue);
                    strErrMsg.append("' does not refer to an existing value of enum '");
                    strErrMsg.append(m_strName);
                    strErrMsg.append("'");
                    srcFile.ThrowParseErr(strErrMsg);
                }
                m_vSynonyms.emplace_back(strCurToken, itVal->m_iOrdinal);
            }
        }
        else if (strCurToken == "Val")
        {
            srcFile.CheckEqualSign();

            //
            //  Let a value object parse itself out. We tell it if we must see an explicit
            //  ordinal based on our type.
            //
            evaliNew.ParseFrom
            (
                srcFile, bExplicitOrdinal(), m_bDoAltVal, m_bText1, m_bText2, m_strName
            );

            // It can't be the same as any existing value or synonym
            if ((itFindValue(evaliNew.m_strName) != m_vValues.end())
            ||  (itFindSynonym(evaliNew.m_strName) != m_vSynonyms.end()))
            {
                std::string strErrMsg("Name '");
                strErrMsg.append(evaliNew.m_strName);
                strErrMsg.append("' is already in use in enum '");
                strErrMsg.append(m_strName);
                strErrMsg.append("'");
                srcFile.ThrowParseErr(strErrMsg);
            }

            //
            //  If this type get an explicit ordinal, make sure that it is not
            //  already taken. They must be unique. And it must be greater than
            //  any so far (so they are in ascending order.)
            //
            if (bExplicitOrdinal())
            {
                if (!m_vValues.empty())
                {
                    if (evaliNew.m_iOrdinal <= iHighestVal)
                    {
                        std::string strErrMsg("Ordinal value '");
                        strErrMsg.append(evaliNew.m_strName);
                        strErrMsg.append("' must be greater than the previous one. Enum=");
                        strErrMsg.append(m_strName);
                        srcFile.ThrowParseErr(strErrMsg);
                    }

                    // If it's not one more than the previous, then not monotonic
                    if (evaliNew.m_iOrdinal != iHighestVal + 1)
                    {
                        m_bIsMonotonic = false;
                    }
                }
                iHighestVal = evaliNew.m_iOrdinal;
            }

            m_vValues.push_back(std::move(evaliNew));
        }
        else
        {
            srcFile.ThrowParseErr("Expected flag or enumeration value block");
        }
    }

    // If the type wasn't explicitly set, then it's assumed standard
    if (m_eType == EEnumTypes::Count)
    {
        m_eType = EEnumTypes::Standard;
    }

    // If no values were defined, that's not valid
    if (m_vValues.empty())
    {
        std::string strErrMsg("No values were defined for enumeration: ");
        strErrMsg.append(m_strName);
        srcFile.ThrowParseErr(strErrMsg);
    }

    // If increment support requested, then can't be bitmapepd and must be monotonic
    if (m_bDoInc && ((m_eType == EEnumTypes::Bitmap) || !m_bIsMonotonic))
    {
        std::string strErrMsg("Increment support is only valid on non-bitmap, monotonic enums");
        srcFile.ThrowParseErr(strErrMsg);
    }

    //
    //  If a standard style, go ahead and number the ordinals sequentially from zero
    //  so we don't have to special case these during output.
    //
    if (m_eType == EEnumTypes::Standard)
    {
        int32_t iOrdinal = 0;
        for (EnumValInfo& evaliCur : m_vValues)
        {
            evaliCur.m_iOrdinal = iOrdinal++;
        }
    }
}


// Find an enum value by its name
std::vector<EnumValInfo>::iterator EnumInfo::itFindValue(const std::string& strToFind)
{
    return std::find_if
    (
        m_vValues.begin()
        , m_vValues.end()
        , [&strToFind](const EnumValInfo& evaliCur) { return evaliCur.m_strName == strToFind; }
    );
}


// Find an enum value by its ordinal value
std::vector<EnumValInfo>::iterator EnumInfo::itFindValueByOrd(const int32_t iToFind)
{
    return std::find_if
    (
        m_vValues.begin()
        , m_vValues.end()
        , [iToFind](const EnumValInfo& evaliCur) { return evaliCur.m_iOrdinal == iToFind; }
    );
}

// Find an enum synonum by its name
std::vector<EnumInfo::KVSPair>::iterator EnumInfo::itFindSynonym(const std::string& strToFind)
{
    return std::find_if
    (
        m_vSynonyms.begin()
        , m_vSynonyms.end()
        , [&strToFind](const KVSPair& pairCur) { return pairCur.first == strToFind; }
    );
}



//
//  At this point the main parsing code has seen the Enums= opening line and
//  knows that this is what is next. So we should see Enum= blocks until we get
//  the end of this overall Enums block.
//
void EnumInfoList::ParseFrom(InputSrc& srcFile)
{
    // A temp list to parse into
    std::vector<EnumInfo> vTmpList;
    EnumInfo enumiNew;
    while (true)
    {
        if (!srcFile.bNextChildOrEnd("Enum", "EndEnums"))
        {
            // We got the end of the Enums block, so we are done
            break;
        }

        // It's an enum definition, so parse it to a temp
        enumiNew.ParseFrom(srcFile);

        // Make sure this name is not already used
        std::vector<EnumInfo>::iterator it = std::find_if
        (
            vTmpList.begin()
            , vTmpList.end()
            , [&enumiNew](const EnumInfo& enumiCur) { return enumiCur.m_strName == enumiNew.m_strName; }
        );
        if (it != vTmpList.end())
        {
            std::string strErrMsg("'");
            strErrMsg.append(enumiNew.m_strName);
            strErrMsg.append("' is already used by another enumeration");
            srcFile.ThrowParseErr(strErrMsg);
        }

        vTmpList.push_back(std::move(enumiNew));
    }

    // It all worked, so store the temp list
    m_vEnumList = std::move(vTmpList);
}

}};



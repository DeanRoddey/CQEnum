#include "CQEnum_Pch.hpp"
#include "CQEnum.hpp"


namespace CQSL { namespace CQEnum {


void ConstInfoList::ParseFrom(InputSrc& srcFile)
{
    static const char* const pszExpectedErr = "Expected constant definition or end of constants";

    //
    //  We are looking for const, constexpr, or external const lines, or the end
    //  of the constants block.
    //
    std::string strType;
    std::vector<std::string> vTokens;
    while (true)
    {
        srcFile.GetIdToken(pszExpectedErr, strType);

        if (strType == "EndConstants")
        {
            break;
        }

        ConstInfo cinfoNew;
        if ((strType == "Const") || (strType == "ConstExpr") || (strType == "ExtConst"))
        {
            if (strType == "Const")
            {
                cinfoNew.m_eType = EConstTypes::Const;
            }
            else if (strType == "ConstExpr")
            {
                cinfoNew.m_eType = EConstTypes::ConstExpr;
            }
            else if (strType == "ExtConst")
            {
                cinfoNew.m_eType = EConstTypes::ExtConst;
            }
            else
            {
                strType = "'";
                strType.append(strType );
                strType.append("' is not a valid constant type");
                srcFile.ThrowParseErr(strType);
            }

            // Have to see an equal sign next
            srcFile.CheckEqualSign();

            //
            //  Then we have a set of three comma separated tokens, which are name, type,
            //  and value.
            //
            srcFile.GetCommaSepValues(vTokens);
            if (vTokens.size() != 3)
            {
                srcFile.ThrowParseErr("A constant definition is a comma separated name, type and value");
            }

            // Make sure the name is unique
            if (m_sNameDupCheck.find(vTokens.at(0)) == m_sNameDupCheck.end()) 
            {
                cinfoNew.m_strName = std::move(vTokens.at(0));
                cinfoNew.m_strType = std::move(vTokens.at(1));
                cinfoNew.m_strValue = std::move(vTokens.at(2));

                // Add this guy's name to the dup check list and then add to the const list
                m_sNameDupCheck.insert(cinfoNew.m_strName);
                m_vConstList.push_back(std::move(cinfoNew));
            }
            else
            {
                std::string strErrMsg("Constant name '");
                strErrMsg.append(vTokens.at(0));
                strErrMsg.append("' is already used");
                srcFile.ThrowParseErr(strErrMsg);
            }
        }
        else
        {
            srcFile.ThrowParseErr(pszExpectedErr);
        }
    }
}


}};

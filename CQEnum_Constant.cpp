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

        if ((strType == "Const") || (strType == "ConstExpr") || (strType == "ExtConst"))
        {
            ConstInfo cinfoNew;
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

            cinfoNew.m_strName = vTokens.at(0);
            cinfoNew.m_strType = vTokens.at(1);
            cinfoNew.m_strValue = vTokens.at(2);

            // Make sure the name is unique
            std::vector<ConstInfo>::const_iterator it = std::find_if
            (
                m_vConstList.begin()
                , m_vConstList.end()
                , [&cinfoNew](const ConstInfo& cinfoCur) { return cinfoNew.m_strName == cinfoCur.m_strName; } 
            );

            if (it != m_vConstList.end())
            {
                std::string strErrMsg("Constant name '");
                strErrMsg.append(cinfoNew.m_strName);
                strErrMsg.append("' is already used");
                srcFile.ThrowParseErr(strErrMsg);

            }
            m_vConstList.push_back(std::move(cinfoNew));
        }
        else
        {
            srcFile.ThrowParseErr(pszExpectedErr);
        }
    }
}


}};

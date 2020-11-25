#pragma once

//
//  This file is just a single place to track all of the data we parse
//  from the definition file. So the lists of constants and enums, plus
//  some overall info like namespaces and export keyword.
//
//  The program creates one of these and then passes it through to all
//  of the parsing code which fills in the members.
//

namespace CQSL { namespace CQEnum {

class CQEnumInfo
{
    public :
        CQEnumInfo() = default;
        ~CQEnumInfo() = default;
        CQEnumInfo(CQEnumInfo&&) = default;

        // Unimplemented
        CQEnumInfo(const CQEnumInfo&) = delete;
        CQEnumInfo& operator=(const CQEnumInfo&) = delete;
        CQEnumInfo& operator=(CQEnumInfo&&) = delete;

        void ParseFrom(InputSrc& srcFile);

        // The list of constants
        ConstInfoList               m_listConsts;

        // The list of enums
        EnumInfoList                m_listEnums;

        int32_t                     m_iVersion = 1;

        // They can provide us with an export keyword to use for DLL based enums
        std::string                 m_strExportMacro;

        //
        //  We pre-build the namespace prefix for the defined namespaces, from
        //  the m_vNamespaces list below. This saves a lot of busy work when
        //  generating the output.
        //
        std::string                 m_strNSPrefix;

        // A list of nested namespaces to put the content in
        std::vector<std::string>    m_vNamespaces;
};

}};

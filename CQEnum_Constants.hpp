#pragma once

//
//  This file handles the constants section of the definition file. So we
//  have a class to store the info for an individual constant and then
//  a list of constants.
//

namespace CQSL { namespace CQEnum {

enum class EConstTypes { Const, ConstExpr, ExtConst, Count };

class ConstInfo
{
    public :
        ConstInfo() noexcept : m_eType(EConstTypes::Count) {}
        ConstInfo(const ConstInfo&) = default;
        ConstInfo(ConstInfo&&) = default;
        ~ConstInfo() = default;

        ConstInfo& operator=(const ConstInfo&) = delete;
        ConstInfo& operator=(ConstInfo&&) = default;

        //
        //  We have to track the constant type (const, constexpr, etc...), the
        //  name, the data type, and the value of the constant. This is all loaded
        //  from the file during parsing of course.
        //
        EConstTypes     m_eType;
        std::string     m_strName;
        std::string     m_strType;
        std::string     m_strValue;
};


// Represents the overall constants definition block
class ConstInfoList
{
    public :
        ConstInfoList() = default;
        ~ConstInfoList() = default;
        ConstInfoList(ConstInfoList&&) = default;

        /// Unimplemented
        ConstInfoList(const ConstInfoList&) = delete;
        ConstInfoList& operator=(const ConstInfoList&) = delete;
        ConstInfoList& operator=(ConstInfoList&&) = delete;

        void ParseFrom(InputSrc& srcFile);

        // We keep a list of const info objects
        std::vector<ConstInfo>  m_vConstList;

        // And for efficient 'name used' checks, we keep a set of constant names
        std::set<std::string>   m_sNameDupCheck;
};


}};

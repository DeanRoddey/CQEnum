//
//  This implements the output generator for C++ (there could be
//  other languages potentially at some point.) We get the CQEnumInfo
//  object into which all of the definition was parsed. 
//
//  Since there could be other languages at some point, the outputs
//  are responsible for creating any output files, since that could
//  differ by language. We also have to be able to tell the main
//  program whether our output files need to be (re)generated, either
//  because one or more of them don't exist or they are out of date
//  wrt to the source definition file.
//
#include "CQEnum_Pch.hpp"
#include "CQEnum.hpp"

namespace CQSL { namespace CQEnum {

namespace
{
    //
    //  A structure we use to represent the values in a local table. And we
    //  generate some lookup functions to search an array of these and find
    //  either the ordinal, the name, or the alt value. The ordinal one assumes
    //  that they are in ascending sorted order. Others have to just do a search.
    //  
    //  Note that, for monotonic ones, they can do direct indexing so they don't
    //  need to have these generated.
    //
    constexpr const char* const pszEnumTableTypes = 
    {
        "enum class ETypes { Arbitrary, Bitmap, Standard };\n\n"
        "struct EnumTableVal\n"
        "{\n"
        "    const wchar_t* const   pszName;\n"
        "    int64_t                iOrdinal;\n"
        "    int32_t                iAltVal;\n"
        "    const wchar_t* const   pszText1;\n"
        "    const wchar_t* const   pszText2;\n"
        "};\n\n"
        
        "const wchar_t* const pszOrdinalToAltText(const  EnumTableVal* const paTable\n"
        "                                        , const uint32_t            uCount\n"
        "                                        , const int64_t             iFind\n"
        "                                        , const uint32_t            uWhichText)\n"
        "{\n"
        "    for (uint32_t uIndex = 0; uIndex < uCount; uIndex++)\n"
        "    {\n"
        "        if (paTable[uIndex].iOrdinal == iFind)\n"
        "        {\n"
        "            return (uWhichText == 1) ? paTable[uIndex].pszText1 : paTable[uIndex].pszText2;\n"
        "        }\n"
        "        if (paTable[uIndex].iOrdinal > iFind)\n"
        "        {\n"
        "            break;\n"
        "        }\n"
        "    }\n"
        "    return nullptr;\n"
        "}\n\n"

        "uint32_t uOrdinalToIndex(  const   EnumTableVal* const paTable\n"
        "                           , const uint32_t            uCount\n"
        "                           , const int64_t             iFind)\n"
        "{\n"
        "    for (uint32_t uIndex = 0; uIndex < uCount; uIndex++)\n"
        "    {\n"
        "        if (paTable[uIndex].iOrdinal == iFind)\n"
        "        {\n"
        "            return uIndex;\n"
        "        }\n"
        "        if (paTable[uIndex].iOrdinal > iFind)\n"
        "        {\n"
        "            break;\n"
        "        }\n"
        "    }\n"
        "    return std::numeric_limits<uint32_t>::max();\n"
        "}\n\n"

        "uint32_t uAltValToIndex(const  EnumTableVal* const paTable\n"
        "                       , const uint32_t            uCount\n"
        "                       , const int32_t             iFind)\n"
        "{\n"
        "    for (uint32_t uIndex = 0; uIndex < uCount; uIndex++)\n"
        "    {\n"
        "        if (paTable[uIndex].iAltVal == iFind)\n"
        "        {\n"
        "            return uIndex;\n"
        "        }\n"
        "    }\n"
        "    return std::numeric_limits<uint32_t>::max();\n"
        "}\n\n"

        "uint32_t uNameToIndex( const   EnumTableVal* const paTable\n"
        "                       , const uint32_t            uCount\n"
        "                       , const wchar_t* const      pszFind)\n"
        "{\n"
        "    for (uint32_t uIndex = 0; uIndex < uCount; uIndex++)\n"
        "    {\n"
        "        if (_wcsicmp(pszFind, paTable[uIndex].pszName) == 0)\n"
        "        {\n"
        "            return uIndex;\n"
        "        }\n"
        "    }\n"
        "    return std::numeric_limits<uint32_t>::max();\n"
        "}\n\n"
    };
}


//
//  Checking to see if we need to output is language specific, so we have to
//  provide this check.
//
bool CQEnumCppOutput::bMustGenerate(const   std::string&    strTarDir
                                    , const std::string&    strBaseName
                                    , const file_time_type& tmSource)
{
    // Generate the paths to our output files (hpp, cpp)
    path pathHdr, pathImpl;
    CreatePaths(strTarDir, strBaseName, pathHdr, pathImpl);

    // If either doesn't exist or is older than the source
    return
    (
        !std::filesystem::exists(pathHdr)
        || (std::filesystem::last_write_time(pathHdr) < tmSource)
        || !std::filesystem::exists(pathImpl)
        || (std::filesystem::last_write_time(pathImpl) < tmSource)
    );
}


//
//  This is called by the main file to let us output the collected data.
//
void
CQEnumCppOutput::GenerateOutput( const   CQEnumInfo&    cqeiSrc
                                , const std::string&    strTarDir
                                , const std::string&    strBaseName)
{
    // Create our target file paths and then open streams for them
    path pathHdr, pathImpl;
    CreatePaths(strTarDir, strBaseName, pathHdr, pathImpl);

    m_strmHdr.open(pathHdr, std::ofstream::out);
    if (m_strmHdr.flags() & std::ofstream::badbit)
    {
        throw std::runtime_error("Could not open the target header file");
    }
    m_strmImpl.open(pathImpl, std::ofstream::out);
    if (m_strmImpl.flags() & std::ofstream::badbit)
    {
        throw std::runtime_error("Could not open the target cpp file");
    }

    // Spit out the opening bits of the two files
    m_strmHdr   << "// This file was auto-generated by Spidl.exe, do not edit\n\n";
    m_strmImpl  << "// This file was auto-generated by Spidl.exe, do not edit\n"
                << "#include <cstdint>\n"
                << "#include <stdexcept>\n"
                << "#include " << pathHdr.filename() << "\n\n";

    for (const std::string& strNS : cqeiSrc.m_vNamespaces)
    {
        m_strmHdr << "namespace " << strNS << " { ";
        m_strmImpl << "namespace " << strNS << " { ";
    }

    // Spit out some types we use in the generated content, inside an anon NS
    m_strmImpl  << " namespace {\n\n" << pszEnumTableTypes << "\n\n";
    m_strmHdr << "\n\n";

    // Generate the constants content
    GenConstants(cqeiSrc);

    // Generate the enumeration content
    GenEnums(cqeiSrc);

    // Close out the namespace in the header
    m_strmHdr << "\n}};\n\n\n";

    // Close off the anonymous namespace of the impl, but not all of them yet
    m_strmImpl << "\n} // Anonymous namespace\n\n\n";

    // Now generate any global header stuff and implementation content
    GenGlobals(cqeiSrc);
    GenImpl(cqeiSrc);

    // And close off the main namespaces in the impl file
    m_strmImpl << "\n}};\n\n\n";

    // Flush both streams and close them and we are done
    m_strmHdr.flush();
    m_strmImpl.flush();
    m_strmHdr.close();
    m_strmImpl.close();
}


// Give a target dir and base output name, generate our C++ specifc output file paths
void CQEnumCppOutput::CreatePaths(  const   std::string&    strTarDir
                                    , const std::string&    strBaseName
                                    ,       path&           pathHdr
                                    ,       path&           pathImpl)
{
    // Create our header and implementation file paths
    std::filesystem::path pathBaseName = strBaseName;
    pathHdr = strTarDir;
    pathHdr /= pathBaseName;
    pathImpl = pathHdr;
    pathHdr += ".hpp";
    pathImpl += ".cpp";
}



// Generate any defined constants
void CQEnumCppOutput::GenConstants(const CQEnumInfo& cqeiSrc)
{
    // To avoid spitting out comments and spacing if there aren't any
    if (cqeiSrc.m_listConsts.m_vConstList.empty())
    {
        return;
    }

    //
    //  Generate the header contents for all of the enums. We do what we can
    //  inline as constexpr stuff.
    //
    for (const ConstInfo& constiCur : cqeiSrc.m_listConsts.m_vConstList)
    {
        m_strmHdr << "    ";
        switch(constiCur.m_eType)
        {
            case EConstTypes::Const :
                m_strmHdr << "static const ";
                break;

            case EConstTypes::ConstExpr :
                m_strmHdr << "constexpr ";
                break;

            case EConstTypes::ExtConst :
                m_strmHdr << cqeiSrc.m_strExportMacro << "extern ";
                break;

            default :
                throw std::runtime_error("Unknown constant type in C++ output generator");
                break;
        };

        m_strmHdr << constiCur.m_strType << " " << constiCur.m_strName;

        if ((constiCur.m_eType == EConstTypes::Const)
        ||  (constiCur.m_eType == EConstTypes::ConstExpr))
        {
            m_strmHdr << " = " << constiCur.m_strValue;
        }

        m_strmHdr << ";\n";
    }
    m_strmHdr << "\n\n";
}


// Generate any defined enums
void CQEnumCppOutput::GenEnums(const CQEnumInfo& cqeiSrc)
{
    // To avoid spitting out comments and spacing if there aren't any
    if (cqeiSrc.m_listEnums.m_vEnumList.empty())
    {
        return;
    }

    //
    //  Generate the header contents for all of the enums. We do what we can
    //  inline as constexpr stuff.
    //
    bool bFirstEnum = true;
    for (const EnumInfo& enumiCur : cqeiSrc.m_listEnums.m_vEnumList)
    {
        if (bFirstEnum)
        {
            bFirstEnum = false;
        }
        else
        {
            m_strmHdr << "\n\n";
        }
        
        m_strmHdr   << "    enum class "
                    << enumiCur.m_strName << " : " << enumiCur.m_strUnderType
                    << "\n    {\n";

        bool bFirstVal = true;
        for (const EnumValInfo& evalCur : enumiCur.m_vValues)
        {
            if (bFirstVal)
            {
                m_strmHdr << "          ";
                bFirstVal = false;
            }
            else
            {
                m_strmHdr << "        , ";
            }
            m_strmHdr   << evalCur.m_strName
                        << " = " << std::to_string(evalCur.m_iOrdinal) << "\n";
        }

        // Do any magic values
        if ((enumiCur.m_eType == EEnumTypes::Arbitary)
        ||  (enumiCur.m_eType == EEnumTypes::Standard))
        {
            m_strmHdr   << "        , Min = "
                        << enumiCur.m_vValues.at(0).m_iOrdinal
                        << "\n        , Max = "
                        << enumiCur.m_vValues.at(enumiCur.m_vValues.size() - 1).m_iOrdinal
                        << "\n        , Count = "
                        << enumiCur.m_vValues.size()
                        << "\n";
        }
        else if (enumiCur.m_eType == EEnumTypes::Bitmap)
        {
            // Build up a mask of all bits
            uint32_t uBits = 0;
            for (const EnumValInfo& evalCur : enumiCur.m_vValues)
            {
                uBits |= static_cast<uint32_t>(evalCur.m_iOrdinal);
            }
            m_strmHdr   << "        , AllBits = 0x"
                        << std::hex << uBits << std::dec << "\n";
        }

        // Do any synonyms
        if (!enumiCur.m_vSynonyms.empty())
        {
            for (const EnumInfo::KVSPair& pairCur : enumiCur.m_vSynonyms)
            {
                m_strmHdr   << "        , "
                            << pairCur.first << " = " << std::to_string(pairCur.second)
                            << "\n";
            }
        }

        m_strmHdr << "    };\n";

        //
        //  If it has alt text, then generate the translation method decls. These go into the
        //  generated namespaces. Same for alt value translation.
        //
        if (enumiCur.m_bText1)
        {
            m_strmHdr   << "    " << cqeiSrc.m_strExportMacro
                        << "const wchar_t* pszEnumToAltText1(const " << cqeiSrc.m_strNSPrefix
                        << enumiCur.m_strName << " eVal);\n";
        }

        if (enumiCur.m_bText2)
        {
            m_strmHdr   << "    " << cqeiSrc.m_strExportMacro
                        << "const wchar_t* pszEnumToAltText2(const " << cqeiSrc.m_strNSPrefix
                        << enumiCur.m_strName << " eVal);\n";
        }

        if (enumiCur.m_bDoAltVal)
        {
            m_strmHdr   << "    " << cqeiSrc.m_strExportMacro
                        << "bool bEnumToAltValue(const " << cqeiSrc.m_strNSPrefix
                        << enumiCur.m_strName << " eVal, int32_t& iToFill);\n";
            m_strmHdr   << "    " << cqeiSrc.m_strExportMacro
                        << "bool bEnumFromAltValue(const int32_t iAltVal, "
                        << cqeiSrc.m_strNSPrefix << enumiCur.m_strName
                        << "& eToFill);\n";
        }

        // We always do the name/value translations both directions
        m_strmHdr   << "    " << cqeiSrc.m_strExportMacro
                    << "const wchar_t* pszEnumToName(const " << cqeiSrc.m_strNSPrefix
                    << enumiCur.m_strName << " eVal);\n";

        m_strmHdr   << "    " << cqeiSrc.m_strExportMacro
                    << "bool bNameToEnum(const wchar_t* const pszText, "
                    << cqeiSrc.m_strNSPrefix << enumiCur.m_strName << "& eToFill);\n";

        // Do an inline one that takes the string object and calls the other version
        m_strmHdr   << "    inline " << cqeiSrc.m_strExportMacro
                    << "bool bNameToEnum(const std::wstring& strText, "
                    << cqeiSrc.m_strNSPrefix << enumiCur.m_strName << "& eToFill)\n"
                    << "    {\n"
                    << "        return bNameToEnum(strText.c_str(), eToFill);\n"
                    << "    }\n";

        //
        //  Do the validity check. If monotonic, we can do a fast inline version. Else
        //  we need the out of line version that does the lookup.
        //
        if (enumiCur.m_bIsMonotonic)
        {
            m_strmHdr   << "    inline bool bIsValidEnumVal(const "
                        << cqeiSrc.m_strNSPrefix << enumiCur.m_strName << " eTest)\n"
                        << "    {\n"
                        << "        return ((eTest >= " << cqeiSrc.m_strNSPrefix << enumiCur.m_strName
                        << "::Min) && (eTest <= " << cqeiSrc.m_strNSPrefix << enumiCur.m_strName
                        << "::Max));\n"
                        << "    }\n";
        }
        else
        {
            m_strmHdr   << "    " << cqeiSrc.m_strExportMacro
                        << "bool bIsValidEnumVal(const " << cqeiSrc.m_strNSPrefix
                        << enumiCur.m_strName << " eTest);\n";
        }

        // If bitmaped, do the bit checks
        if (enumiCur.m_eType == EEnumTypes::Bitmap)
        {
            m_strmHdr   << "    inline bool bAllEnumBitsOn(const " 
                        << cqeiSrc.m_strNSPrefix << enumiCur.m_strName << " eTest, const "
                        << cqeiSrc.m_strNSPrefix << enumiCur.m_strName << " eBits)\n    {\n"
                        << "        return (static_cast<" << enumiCur.m_strUnderType << ">(eTest) & "
                        << "static_cast<" << enumiCur.m_strUnderType << ">(eBits)) == static_cast<"
                        << enumiCur.m_strUnderType << ">(eBits);\n    }\n";

            m_strmHdr   << "    inline bool bAnyEnumBitsOn(const " 
                        << cqeiSrc.m_strNSPrefix << enumiCur.m_strName << " eTest, const "
                        << cqeiSrc.m_strNSPrefix << enumiCur.m_strName
                        << " eBits)\n    {\n        return (static_cast<"
                        << enumiCur.m_strUnderType << ">(eTest) & "
                        << "static_cast<" << enumiCur.m_strUnderType << ">(eBits)) != 0;\n    }\n";
        }
    }

    //  Generate the lookup tables and supporting code in the impl file
    for (const EnumInfo& enumiCur : cqeiSrc.m_listEnums.m_vEnumList)
    {
        m_strmImpl  << "struct\n{\n    ETypes         eType"
                    << ";\n    EnumTableVal   aevalTable["
                    << enumiCur.m_vValues.size()
                    << "];\n} EnumTable_" << enumiCur.m_strName << " =\n{\n    ";

        switch(enumiCur.m_eType)
        {
            case EEnumTypes::Arbitary : m_strmImpl << "ETypes::Arbitrary"; break;
            case EEnumTypes::Bitmap : m_strmImpl << "ETypes::Bitmap"; break;
            case EEnumTypes::Standard : m_strmImpl << "ETypes::Standard"; break;

            default :
                throw std::runtime_error("Internal: Unknown enum type in local table");
                break;
        };

        bool bFirst = true;
        m_strmImpl << ",\n    {\n";
        for (const EnumValInfo& evalCur : enumiCur.m_vValues)
        {
            if (!bFirst)
            {
                m_strmImpl << "      , { ";
            }
            else
            {
                m_strmImpl << "        { ";
                bFirst = false;
            }

            m_strmImpl  << "L\"" << evalCur.m_strName << "\", "
                        << evalCur.m_iOrdinal << ", "
                        << evalCur.m_iAltValue << ", ";

            m_strmImpl << "L\"" << evalCur.m_strText1 << "\", ";
            m_strmImpl << "L\"" << evalCur.m_strText2 << "\"";
            
            m_strmImpl << " }\n";
        }

        m_strmImpl << "\n    }\n};\n\n";
    }
}


// Generate any defined global operators
void CQEnumCppOutput::GenGlobals(const CQEnumInfo& cqeiSrc)
{
    // Global header stuff we need outside of the namespaces
    for (const EnumInfo& enumiCur : cqeiSrc.m_listEnums.m_vEnumList)
    {
        if (enumiCur.m_bDoInc)
        {
            m_strmHdr   << "inline " << cqeiSrc.m_strNSPrefix << enumiCur.m_strName
                        << "& operator++(" << cqeiSrc.m_strNSPrefix << enumiCur.m_strName
                        << "& eVal)\n{\n    if (eVal < " << cqeiSrc.m_strNSPrefix << enumiCur.m_strName
                        << "::Count)\n    {\n        eVal = static_cast<"
                        << cqeiSrc.m_strNSPrefix << enumiCur.m_strName
                        << ">(static_cast<" << enumiCur.m_strUnderType
                        << ">(eVal) + 1);\n    }\n    return eVal;\n}\n";

            m_strmHdr   << "inline " << cqeiSrc.m_strNSPrefix << enumiCur.m_strName
                        << " operator++(" << cqeiSrc.m_strNSPrefix << enumiCur.m_strName
                        << "& eVal, int)\n{\n    const " << cqeiSrc.m_strNSPrefix << enumiCur.m_strName
                        << " eRet = eVal;\n"
                        << "    if (eVal < " << cqeiSrc.m_strNSPrefix << enumiCur.m_strName
                        << "::Count)\n    {\n        eVal = static_cast<"
                        << cqeiSrc.m_strNSPrefix << enumiCur.m_strName
                        << ">(static_cast<" << enumiCur.m_strUnderType
                        << ">(eVal) + 1);\n    }\n    return eRet;\n}\n";
        }

        // If a bitmap type, generate bitwise ops and testing methods
        if (enumiCur.m_eType == EEnumTypes::Bitmap)
        {
            m_strmHdr   << "constexpr " << cqeiSrc.m_strNSPrefix << enumiCur.m_strName
                        << " operator|(const " << cqeiSrc.m_strNSPrefix << enumiCur.m_strName
                        << " eLHS, const " << cqeiSrc.m_strNSPrefix << enumiCur.m_strName
                        << " eRHS)\n{\n    return static_cast<"
                        << cqeiSrc.m_strNSPrefix << enumiCur.m_strName
                        << ">(static_cast<" << enumiCur.m_strUnderType
                        << ">(eLHS) | static_cast<" << enumiCur.m_strUnderType
                        << ">(eRHS));\n}\n";

            m_strmHdr   << "constexpr " << cqeiSrc.m_strNSPrefix << enumiCur.m_strName
                        << " operator&(const " << cqeiSrc.m_strNSPrefix << enumiCur.m_strName
                        << " eLHS, const " << cqeiSrc.m_strNSPrefix << enumiCur.m_strName
                        << " eRHS)\n{\n    return static_cast<"
                        << cqeiSrc.m_strNSPrefix << enumiCur.m_strName
                        << ">(static_cast<" << enumiCur.m_strUnderType
                        << ">(eLHS) & static_cast<" << enumiCur.m_strUnderType
                        << ">(eRHS));\n}\n";

            m_strmHdr   << "constexpr " << cqeiSrc.m_strNSPrefix << enumiCur.m_strName
                        << " operator|=(" << cqeiSrc.m_strNSPrefix << enumiCur.m_strName
                        << "& eLHS, const " << cqeiSrc.m_strNSPrefix << enumiCur.m_strName
                        << " eRHS)\n{\n    eLHS = static_cast<"
                        << cqeiSrc.m_strNSPrefix << enumiCur.m_strName
                        << ">(static_cast<" << enumiCur.m_strUnderType
                        << ">(eLHS) | static_cast<" << enumiCur.m_strUnderType
                        << ">(eRHS));\n    return eLHS;\n}\n";

            m_strmHdr   << "constexpr " << cqeiSrc.m_strNSPrefix << enumiCur.m_strName
                        << " operator&=(" << cqeiSrc.m_strNSPrefix << enumiCur.m_strName
                        << "& eLHS, const " << cqeiSrc.m_strNSPrefix << enumiCur.m_strName
                        << " eRHS)\n{\n    eLHS = static_cast<"
                        << cqeiSrc.m_strNSPrefix << enumiCur.m_strName
                        << ">(static_cast<" << enumiCur.m_strUnderType
                        << ">(eLHS) & static_cast<" << enumiCur.m_strUnderType
                        << ">(eRHS));\n    return eLHS;\n}\n";
        }

        m_strmHdr << "\n\n";
    }
}


// Generate the implementation stuff into the cpp file
void CQEnumCppOutput::GenImpl(const CQEnumInfo& cqeiSrc)
{
    // We only need to do external type constants in the cpp file
    m_strmImpl << "// External constants\n";
    for (const ConstInfo& constiCur : cqeiSrc.m_listConsts.m_vConstList)
    {
        if (constiCur.m_eType == EConstTypes::ExtConst)
        {
            m_strmImpl  << constiCur.m_strType << " " << constiCur.m_strName << " = "
                        << constiCur.m_strValue << ";\n";
        }
    }
    m_strmImpl << "\n\n";

    // Do the implementations of the public stuff that isn't inlined
    m_strmImpl << "// Enumeration method implementations\n";
    for (const EnumInfo& enumiCur : cqeiSrc.m_listEnums.m_vEnumList)
    {
        // We always do the name/value translations
        m_strmImpl  << "bool bNameToEnum(const wchar_t* const pszName, "
                    << cqeiSrc.m_strNSPrefix << enumiCur.m_strName << "& eToFill)\n"
                    << "{\n"
                    << "    const uint32_t uIndex = uNameToIndex(EnumTable_"
                    << enumiCur.m_strName << ".aevalTable, "
                    << enumiCur.m_vValues.size() << ", pszName);\n"
                    << "    if (uIndex >= " << enumiCur.m_vValues.size() << ")\n"
                    << "    {\n"
                    << "        return false;\n"
                    << "    }\n"
                    << "    eToFill = static_cast<" << cqeiSrc.m_strNSPrefix << enumiCur.m_strName
                    << ">(EnumTable_" << enumiCur.m_strName << ".aevalTable[uIndex].iOrdinal);\n"
                    << "    return true;\n"
                    << "}\n";

        m_strmImpl  << "const wchar_t* pszEnumToName(const " << cqeiSrc.m_strNSPrefix
                    << enumiCur.m_strName << " eVal)\n"
                    << "{\n"
                    << "    const uint32_t uIndex = uOrdinalToIndex(EnumTable_"
                    << enumiCur.m_strName << ".aevalTable, " << enumiCur.m_vValues.size()
                    << ", static_cast<int64_t>(eVal));\n"
                    << "    if (uIndex >= " << enumiCur.m_vValues.size() << ")\n"
                    << "    {\n"
                    << "        return nullptr;\n"
                    << "    }\n"
                    << "    return EnumTable_" << enumiCur.m_strName << ".aevalTable[uIndex].pszName;\n"
                    << "}\n";

        //
        //  We always do the valid value check. If it's a monotonic one, we can just check
        //  that the value is between the min/max inclusive, which is done inline. Else we have
        //  to call the lookup helper method and need and out of line one.
        //
        if (!enumiCur.m_bIsMonotonic)
        {
            m_strmImpl  << "bool bIsValidEnumVal(const "
                        << cqeiSrc.m_strNSPrefix << enumiCur.m_strName << " eTest)\n"
                        << "{\n"
                        << "    const uint32_t uIndex = uOrdinalToIndex(EnumTable_"
                        << enumiCur.m_strName << ".aevalTable, " << enumiCur.m_vValues.size()
                        << ", static_cast<int64_t>(eTest));\n"
                        << "    return (uIndex < " << enumiCur.m_vValues.size() << ");\n"
                        << "}\n";
        }

        if (enumiCur.m_bText1)
        {
            GenTextXlatImpl(1, enumiCur, cqeiSrc);
        }

        if (enumiCur.m_bText2)
        {
            GenTextXlatImpl(2, enumiCur, cqeiSrc);
        }

        if (enumiCur.m_bDoAltVal)
        {
            m_strmImpl  << "bool bEnumToAltValue(const " << cqeiSrc.m_strNSPrefix
                        << enumiCur.m_strName << " eVal, int32_t& iToFill)\n{\n"
                        << "    const uint32_t uIndex = uOrdinalToIndex(EnumTable_"
                        << enumiCur.m_strName << ".aevalTable"
                        << ", " << enumiCur.m_vValues.size() << ", static_cast<int64_t>(eVal));\n"
                        << "    if (uIndex >= " << enumiCur.m_vValues.size() << ")\n"
                        << "    {\n"
                        << "        return false;\n"                        
                        << "    }\n"
                        << "    iToFill = EnumTable_" << enumiCur.m_strName << ".aevalTable[uIndex].iAltVal;\n"
                        << "    return true;\n"
                        << "}\n";

            m_strmImpl  << "bool bEnumFromAltValue(const int32_t iAltVal, "
                        << cqeiSrc.m_strNSPrefix << enumiCur.m_strName << "& eToFill)\n"
                        << "{\n"
                        << "    const uint32_t uIndex = uAltValToIndex(EnumTable_"
                        << enumiCur.m_strName << ".aevalTable"
                        << ", " << enumiCur.m_vValues.size() << ", iAltVal);\n"
                        << "    if (uIndex >= " << enumiCur.m_vValues.size() << ")\n"
                        << "    {\n"
                        << "        return false;\n"
                        << "    }\n"
                        << "    eToFill = static_cast<" << cqeiSrc.m_strNSPrefix << enumiCur.m_strName
                        << ">(EnumTable_" << enumiCur.m_strName << ".aevalTable[uIndex].iOrdinal);\n"
                        << "    return true;\n"
                        << "}\n";
        }
    }
}


//
//  Since we have more than one text value, this avoids a lot of redundancy. It will
//  generate output for the first or second text value, based on whether uTextNum is 1
//  or 2.
//
void CQEnumCppOutput::GenTextXlatImpl(const uint32_t        uTextNum
                                    , const EnumInfo&       enumiSrc
                                    , const CQEnumInfo&     cqeiSrc)
{
    m_strmImpl  << "const wchar_t* pszEnumToAltText" << uTextNum << "(const " << cqeiSrc.m_strNSPrefix
                << enumiSrc.m_strName << " eVal)\n{    \n    uint32_t uIndex = 0;\n";

    //
    //  If a monotonic one, we can do a simple validation and direct index.
    //  Else we have to just look for it.
    //
    if (enumiSrc.m_bIsMonotonic)
    {
        m_strmImpl  << "    if ((eVal < " << cqeiSrc.m_strNSPrefix
                    << enumiSrc.m_strName << "::Min) || (eVal > "
                    << cqeiSrc.m_strNSPrefix << enumiSrc.m_strName
                    << "::Max))\n    {\n        throw std::runtime_error(\""
                    << "Invalid value for '" << cqeiSrc.m_strNSPrefix << enumiSrc.m_strName
                    << "' enumeration\");\n    }\n"
                    << "    uIndex = static_cast<uint32_t>(eVal);\n";
    }
    else
    {
        // We have to use a lookup helper
        m_strmImpl  << "    uIndex = uOrdinalToIndex(EnumTable_" << enumiSrc.m_strName << ".aevalTable, "
                    << enumiSrc.m_vValues.size() << ", static_cast<int64_t>(eVal));\n"
                    << "    if (uIndex >= " << enumiSrc.m_vValues.size() << ")\n"
                    << "    {\n"
                    << "        throw std::runtime_error(\"Invalid value for '" << cqeiSrc.m_strNSPrefix
                    << enumiSrc.m_strName << "' enumeration\");\n    }\n";
    }

    m_strmImpl << "    return EnumTable_" << enumiSrc.m_strName << ".aevalTable[uIndex].pszText"
             << uTextNum << ";\n";
    m_strmImpl << "}\n\n";
}

}};


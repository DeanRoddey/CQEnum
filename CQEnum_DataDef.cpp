#include "CQEnum_Pch.hpp"
#include "CQEnum.hpp"

namespace CQSL { namespace CQEnum {

void CQEnumInfo::ParseFrom(InputSrc& srcFile)
{
    // Check the main file info stuff, so File= first
    srcFile.bCheckNextId("File", "Expected the File= block to be first in the file", true, true);

    // We have to see the version first
    m_iVersion = srcFile.iCheckSignedValue("Version", "Expected the Version attribute");
    if (m_iVersion != 1)
    {
        srcFile.ThrowParseErr("The version number must be equal to 1");
    }

    // We expect to see a namespaces value next
    srcFile.bCheckNextId("Namespaces", "Expected to see Namespaces= here", true, true);
    srcFile.GetSpacedValues(m_vNamespaces);

    // We can have an export macro
    if (srcFile.bCheckNextId("ExportMacro", "Expected to see ExportMacro= here", true, false))
    {
        srcFile.GetIdToken("Expected to see export macro name here", m_strExportMacro);

        // Add a space after it so that we don't ahve to do that every time we output it
        m_strExportMacro.push_back(' ');
    }

    // We have to see the end of the file block
    srcFile.CheckBlockEnd("File");

    // If we have a constants block, parse it
    if (srcFile.bCheckNextId("Constants", "Expected Constants=, Enums=", true, false))
    {
        m_listConsts.ParseFrom(srcFile);
    }

    // If we have an enums block, parse it
    if (srcFile.bCheckNextId("Enums", "Expected Constants=, Enums=", true, false))
    {
        m_listEnums.ParseFrom(srcFile);
    }


    //
    //  Pre-build up the namespace prefix for the user's defined namespaces. This is used
    //  a lot so it saves a lot of grunt work.
    //
    m_strNSPrefix.clear();
    for (const std::string& strNS : m_vNamespaces)
    {
        m_strNSPrefix.append(strNS);
        m_strNSPrefix.append("::");
    }
}

}};

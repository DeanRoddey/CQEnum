//
//  This is a simple test application that defines a .cqenum file that
//  will be invoked on build to generate the Test.cpp and Test.hpp files.
//  These contain the generated support for all of the enumerations and
//  constants defined in the .cqenum file. 
//
//  This test file then includes those. The actual code of this test
//  just exercises some basic functionality for the enums that it knows
//  was generated.
//
//  Obviously more tests should be provided here eventually. This is just
//  doing the most basic checks.
//
#include <iostream>
#include "Test.hpp"

using namespace CQSL::Test;

int main()
{
    // A standard enum. Start with it on the min value
    TestEnum eStd = TestEnum::Min;

    // It should be valid now
    if (!bIsValidEnumVal(eStd))
    {
        std::wcout << L"Enum value should be valid" << std::endl;
    }

    // Iterate through the values and output their first alt text value
    while (eStd <= TestEnum::Max)
    {
        std::wcout << pszEnumToAltText1(eStd) << L"\n";
        ++eStd;
    }
    std::wcout  << std::endl;

    // Shouldn't be valid now since we ran it up to Count
    if (bIsValidEnumVal(eStd))
    {
        std::wcout << L"Enum value should not be valid" << std::endl;
    }

    // Translate an enum value to the name of the value
    std::wcout  << L"Name for Value1: " << pszEnumToName(TestEnum::Value1)
                << std::endl;

    // Go the other way, name back to enum value
    if (bNameToEnum(L"Value2", eStd))
    {
        if (eStd != TestEnum::Value2)
        {
            std::wcout << L"Name convered to wrong enum value" << std::endl;
        }
        else
        {
            std::wcout << L"Value2 name translated to correct enum value" << std::endl;
        }
    }
    else
    {
        std::wcout << L"Failed to convert name to enum value" << std::endl;
    }

    // A bitmapped enum. OR some of the values together
    TestBmp eBmp = TestBmp::Value1;
    eBmp |= TestBmp::Value2;
    eBmp = eBmp | TestBmp::Value3;

    // Test that one of the bits we OR'd in is set
    if (bAllEnumBitsOn(eBmp, TestBmp::Value2))
    {
        std::wcout << L"Value2 bit was correctly set" << std::endl;
    }
    else
    {
        std::wcout << L"Value2 bit was not correctly set" << std::endl;
    }

    // Translates one of the values to it's alt2 text
    std::wcout << pszEnumToAltText2(TestBmp::Value1) << L"\n";

    // Translate from the alt numeric value to the enumerated value
    if (!bEnumFromAltValue(102, eBmp))
    {
        std::wcout << L"Could not translate alt value to enum" << std::endl;
    }
     else if (eBmp != TestBmp::Value2)
    {
        std::wcout << L"Alt value translated to wrong enum" << std::endl;
    }
    else
    {
        std::wcout << L"Alt value was translated to correct enum" << std::endl;
    }


    // Test exported constant
    std::wcout << strTest << std::endl;
}

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
//  Obviously more tests could be provided here eventually. This is just
//  doing some basic tests to make sure that the generation is correct.
//
#include <string>
#include <iostream>
#include "Test.hpp"

using namespace CQSL::Test;


int main()
{
    std::wcout << L"Starting CQEnum tests..." << std::endl;

    // A standard enum. Start with it on the min value
    TestEnum eStd = TestEnum::Min;

    // It should be valid now
    if (!bIsValidEnumVal(eStd))
    {
        std::wcout << L"Enum value should be valid" << std::endl;
    }

    // Iterate through the values and output their first text value
    unsigned int uOrdVal = 0;
    std::wstring strCompVal;
    while (eStd <= TestEnum::Max)
    {
        strCompVal = L"The text for value ";
        strCompVal.append(std::to_wstring(uOrdVal + 1));
        if (std::wstring(pszEnumToAltText1(eStd)) != strCompVal)
        {
            std::wcout << L"Invalid std enum to text 1 translation" << std::endl;            
            break;
        }
        ++eStd;
        ++uOrdVal;
    }
    std::wcout  << std::endl;

    // Do the same but for the text2 value
    uOrdVal = 0;
    eStd = TestEnum::Min;
    while (eStd <= TestEnum::Max)
    {
        strCompVal = L"The text2 for value ";
        strCompVal.append(std::to_wstring(uOrdVal + 1));
        if (std::wstring(pszEnumToAltText2(eStd)) != strCompVal)
        {
            std::wcout << L"Invalid std enum to text 2 translation" << std::endl;            
            break;
        }
        ++eStd;
        ++uOrdVal;
    }

    // Shouldn't be valid now since we ran it up to Count
    if (bIsValidEnumVal(eStd))
    {
        std::wcout << L"Enum value should not be valid" << std::endl;
    }

    // Translate an enum value to the name of the value
    if (std::wstring(pszEnumToName(TestEnum::Value1)) != L"Value1")
    {
        std::wcout  << L"The name for value 1 is incorrect"
                    << std::endl;
    }

    // Go the other way, name back to enum value
    if (bNameToEnum(L"Value2", eStd))
    {
        if (eStd != TestEnum::Value2)
        {
            std::wcout << L"Name convered to wrong enum value" << std::endl;
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

    // Test all of the bits we OR'd in is set
    if (!bAllEnumBitsOn(eBmp, TestBmp::Value1))
    {
        std::wcout << L"Value1 bit was not correctly set" << std::endl;
    }
    if (!bAllEnumBitsOn(eBmp, TestBmp::Value2))
    {
        std::wcout << L"Value2 bit was not correctly set" << std::endl;
    }
    if (!bAllEnumBitsOn(eBmp, TestBmp::Value3))
    {
        std::wcout << L"Value3 bit was not correctly set" << std::endl;
    }

    // It should also equal the all bits value now since we turned them all on
    if (eBmp != TestBmp::AllBits)
    {
        std::wcout << L"All enum bits were not correctly enabled" << std::endl;
    }

    // This should not be a valid value since it's a combination
    if (bIsValidEnumVal(eBmp))
    {
        std::wcout << L"Enum bmp value should not be valid" << std::endl;
    }

    // Force two of the bits off
    eBmp &= ~(TestBmp::Value1 | TestBmp::Value3);
    if (eBmp != TestBmp::Value2)
    {
        std::wcout << L"Tilde operator failed" << std::endl;
    }

    // Translate one of this guy's values to its text1 and text2 values
    if (std::wstring(pszEnumToAltText1(TestBmp::Value3)) != L"The text for value 3")
    {
        std::wcout << L"Invalid enum to text 1 translation" << std::endl;            
    }

    if (std::wstring(pszEnumToAltText2(TestBmp::Value2)) != L"The text2 for value 2")
    {
        std::wcout << L"Invalid bmp enum to text 2 translation" << std::endl;            
    }

    // Translate from the alt numeric value to the enumerated value
    if (!bEnumFromAltValue(102, eBmp))
    {
        std::wcout << L"Could not translate alt value to enum" << std::endl;
    }
    else if (eBmp != TestBmp::Value2)
    {
        std::wcout << L"Alt value translated to wrong enum" << std::endl;
    }


    // Test inlined constants
    if (uTest != 10)
    {
        std::wcout << L"uTest value should be 10 but got " << uTest << std::endl;
    }

    if (wcscmp(pszTest, L"The value of the raw string") != 0)
    {
        std::wcout  << L"Got invalid value for pszTest constant (\""
                    << pszTest << L"\")" << std::endl;
    }

    // Test the out of line constants
    if (strTest != L"The value of the string object")
    {
        std::wcout  << L"Got invalid value for strTest constant (\""
                    << strTest << L"\")" << std::endl;
    }

    std::wcout << L"Tests completed" << std::endl;
}

# CQEnum - A code generator to make C++ enums vastly more powerful

## What It Is

CQEnum is a code generator that you can use in your C++ projects. It allows you to define enums and constants in a 'definition file' using a simple structured text format. CQEnum will parse this file and spit out an hpp and cpp file that implements a lot of very useful functionality that turns C++ enums from semi-useful to extremely powerful.

Amongst the things it will do for you:

- Supports three types of enums. Standard enums are 0 based and contiguous, arbitrary don't start at zero and may or may not be contiguous, and bitmapped enums are what you would expect where each value defines a bit in a set of bits.
- For contiguous enums it can generate pre/post increment operators if you ask, so you can use them in loops and such.
- For bitmapped enums it will generate &, |, &=, |= and ~ operators for you, to make it easy to use them as bit masks without casting. Also generates bit testing methods.
- You can define one or two text translations for each enum and translate from enum value to text or from text to enum value.
- You can define an alternate numerical value and translate between the enum value and numerical value
- Defines a validity check to make it easy to validate that an enum has a legal value in it.
- Methods to get the ordinal of a value or create a value from an ordinal.
- You can define synonym values that refer to other enum values, to allow you to define sub-ranges within the full range of values.
- It also allows you to define various types of constants. This is more intended towards potential support for outputing other than just C++ content, i.e. to generate some cross platform content.
- You provide a set of optional nested namespaces and it will generate the content in those namespaces, so the output will naturally show up in whatever namespace your project uses.
- You can provide an export keyword that it will apply to any non-inlined stuff that needs it, to make the generated content useful in loadable libraries like DLLs.

## Invoking CQEnum

CQEnum is very easy to invoke, just provide the source definition file and the target directory. Convention is to use the .cqenum extension but any can be used since you have to indicate the extension anyway. CQEnum will only generate new output if the target files are out of date with respect to the source definition file.

So an example invocation might be this, assuming that it is running in the project's top level directory presumably:

    CQEnum .\Src\MyEnums.cqenum .\Src\

The output will be an hpp and cpp file that have the same base name as the source definition file. So if you pass it MyEnums.cqenum, you will get a MyEnums.hpp and MyEnums.cpp file spit out to the target directory.

## Building CQEnum

CQEnum is quite simple, just a handful of cpp files, so it would be trivial to just build it with a single line batch file in many cases. Otherwise create whatever sort of make file you need for it. It should be trivial to create. 

A Visual Studio solution file is included in the repository if you use that IDE. I don't generally use it but did so for this little utility since it's not part of my regular code base, where I use my own build system.

## Definition Syntax

The syntax for the definition files is a simple line oriented text file. So it doesn't depend on any parsing library and the like. It is composed of a set blocks that define information required for code generation.

Here is a simple example, which will be broken down below:

    ;
    ; This is a test definition file that will cause the files
    ; Test.hpp and Test.cpp to be generated. The test app includes
    ; these files and tests the generated functionality.
    ;

    ; File block is required and must at least indicate the version
    File=
        Version=1
        Namespaces=CQSL Test
        ExportMacro=MYPROJEXP
    EndFile

    Constants=

        ConstExpr=uTest, unsigned int, 10
        Const=pszTest, wchar_t* const, L"The value of the raw string"
        ExtConst=strTest, std::wstring, L"The value of the string object"

    EndConstants

    Enums=

        Enum=TestEnum
            Type=Standard
            Flags=Text1 Text2 Inc

            Val=Value1
                Text1="The text for value 1"
                Text2="The text2 for value 1"
            EndVal

            Val=Value2
                Text1="The text for value 2"
                Text2="The text2 for value 2"
            EndVal

            Val=Value3
                Text1="The text for value 3"
                Text2="The text2 for value 3"
            EndVal

            Synonym=Syn1:Value3

        EndEnum


        Enum=TestBmp
            Type=Bitmap
            UnderType=unsigned short
            Flags=Text1 Text2 AltVal

            Val=Value1
                Text1="The text for value 1"
                Text2="The text2 for value 1"
                Ordinal=0x0001
                AltVal=101
            EndVal

            Val=Value2
                Text1="The text for value 2"
                Text2="The text2 for value 2"
                Ordinal=0x0002
                AltVal=102
            EndVal

            Val=Value3
                Text1="The text for value 3"
                Text2="The text2 for value 3"
                Ordinal=0x0004
                AltVal=0x67
            EndVal

            Synonym=Syn1:Value3
            Synonym=Syn2:Value1,Value3

        EndEnum

    EndEnums

### File Block

The File= block indicates the version this file is supposed to be in, with the only available version currently being 1. You can also optionally indicate a set of nested namespaces to generate the content into. If none are provided it won't be namespace based but you should always do so. And you can indicate an export 'macro' that will be applied to any out of line content that should be exported. This is for use in loadable libraries and wouldn't be used in an executable.

### Constants Block

The Constants= block is used for defining constants and may not be used if you don't want to. It supports inline const values, constexpr values and 'external' (out of line) constants. For each constant you provide the name, the type, and the value to use. External constants will get the export macro (from the File= block) if one is defined. String values must be quoted.

### Enums Block

The Enums= block is the primary block and just contains a list of Enum= blocks.

### Enum Block

Each Enum= block defines a single enumeration. It begins with a set of optional attributes, followed by one or more value definitions (and optinally at the end synonym values.)

**Attributes** 

If present, attributes must come first. These are the available attributes, with the default value assumed if not present being the first one where applicable.

* Type=[Standard | Arbitrary | Bitmap]
* Undertype=[int for Standard/Arbitrary types and unsigned int for Bitmap | any other valid base enum type]
* Flags=[one or more valid flags which are listed below]

The basic types were discussed above, but just for reference:

* Standard = Automatically numbered contiguously from 0, so you can't indicate ordinal values.
* Arbitrary = You must indicate ordinal values. If they happen to be contiguous values, then you can still ask for increment support, but they don't have to be contiguous. These are slightly less efficient since they require more lookups whereas standard ones can more often use direct indexed access.
* Bitmap = You must provide ordinal values. Though not enforced, each one should represent one bit, and of course they should be legal for the underlying type. Use Synonyms below to define combination of bits.

The available flags are:

* Text1 = Each value must provide a Text1 value for text/value translation
* Text2 = Each value must provide a Text2 value for text/value translation
* Inc = Pre/post incremental operators should be generated (only valid for contiguous enums.)
* AltVal = Each value must provide an alternate numeric value

**Values**

Each enum must have at least one Val= block to define at least one enumerated value, and of course generally they have more than one or it's probably not worth being an enumeration. Following the = sign, is the name of that value. Within the blody of the Val= block you must provide any information that you have told the parser to expect. The Flags= attribute above defines a set of values that you must provide (text 1/2, and alt numeric value.) Also, if the type is Arbitrary or Bitmap you must provide an ordinal value.

Values cannot be duplicates, so the parser will reject any duplicates. Use Synonyms (see below) to provide alternate names for values.

**Synonyms**

You can optionally define synonyms. These refer to the values by name, and will have the same ordinal as the values they reference. Synonyms are for defining sub-ranges within the enum values in cases where the limits of that range may change over time and you don't want to have to go update every place where those start/end limits are used. You can just update the synonyms to point to new values.

For Standard and Arbitrary enums the syntax is "synname:referencedvalue". Synname becomes the name of the synonum and referencedvalue must be the name of a regular value. The synonym will be given the same ordinal as the referenced value.

For Bitmap enums, the syntax is "synname:refval1[,refval2]+", So it can be the same as non-bitmaps ones above, or it can reference multiple, comma separated values. The resulting ordinal will contain all of the bits of the referenced values, so it is a convenient way to create convenient combinations of bits.

## Magic Values

This tool will create some magic enumeration values for you, which are used both internally by the generated code and by you in most cases because they are very useful. These vary by type.

* ::Min and ::Max - For Standard types or Arbitrary types that are contiguous, min/max values are generated. These can be used for looping through all values or test values against limits.
* ::Count - For Standard types a Count value is generated, which will will have an ordinal one greater than the last value. This is often more convenient for loops or testing.
* ::AllBits - For Bitmap types, this value will contain all of the bits.
* ::NoBits - For Bitmap types, this value will contain none of the bits, i.e. be zero. This is convenient for a 'not set' value.

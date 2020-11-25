#pragma once

//
//  This file defines our output generator. This one is for C++, though there
//  could potentially be others at some point. So there's a base class and a
//  a Cpp derivative. It gets the main enum info class that has all of the stuff
//  parsed from the definition file, plus a target directory and base name for
//  the output file (which can be created appropriately for the language.)
//

namespace CQSL { namespace CQEnum {

using std::filesystem::file_time_type, std::filesystem::path;

class CQEnumOutputIntf
{
    public :
        virtual ~CQEnumOutputIntf() = default;

        // Unimplemented
        CQEnumOutputIntf(const CQEnumOutputIntf&) = delete;
        CQEnumOutputIntf(CQEnumOutputIntf&&) = delete;
        CQEnumOutputIntf& operator=(const CQEnumOutputIntf&) = delete;
        CQEnumOutputIntf& operator=(CQEnumOutputIntf&&) = delete;

        virtual bool bMustGenerate
        (
            const   std::string&            strTarDir
            , const std::string&            strBaseName
            , const file_time_type&         tmSource
        ) = 0;

        virtual void GenerateOutput
        (
            const   CQEnumInfo&             cqeiSrc
            , const std::string&            strTarDir
            , const std::string&            strBaseName
        ) = 0;


    protected :
        CQEnumOutputIntf() = default;
};


class CQEnumCppOutput : public CQEnumOutputIntf
{
    public :
        CQEnumCppOutput() = default;
        ~CQEnumCppOutput() = default;

        // Unimplemented
        CQEnumCppOutput(const CQEnumCppOutput&) = delete;
        CQEnumCppOutput(CQEnumCppOutput&&) = delete;
        CQEnumCppOutput& operator=(const CQEnumCppOutput&) = delete;
        CQEnumCppOutput& operator=(CQEnumCppOutput&&) = delete;

        bool bMustGenerate
        (
            const   std::string&            strTarDir
            , const std::string&            strBaseName
            , const file_time_type&         tmSource
        )   final;

        void GenerateOutput
        (
            const   CQEnumInfo&             cqeiSrc
            , const std::string&            strTarDir
            , const std::string&            strBaseName
        )   final;


    private :
        void CreatePaths
        (
            const   std::string&            strTarDir
            , const std::string&            strBaseName
            ,       path&                   pathHdr
            ,       path&                   pathImpl
        );

        // Helpers to generate the various types of output
        void GenConstants
        (
            const   CQEnumInfo&             cqeiSrc
        );

        void GenEnums
        (
            const   CQEnumInfo&             cqeiSrc
        );

        void GenGlobals
        (
            const   CQEnumInfo&             cqeiSrc
        );

        void GenImpl
        (
            const   CQEnumInfo&             cqeiSrc
        );
      
        void GenTextXlatImpl
        (
            const   uint32_t                uTextNum
            , const EnumInfo&               enumiSrc
            , const CQEnumInfo&             cqeiSrc
        );


        // We need a stream for the header and for the impl file
        std::ofstream   m_strmHdr;
        std::ofstream   m_strmImpl;
};

}};


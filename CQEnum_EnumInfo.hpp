#pragma once

//
//  This file stores the info about the defined enumerations that we
//  parse from the definition file. So we need the info about a single
//  enum value. Then the info about an enum (which has a list of the
//  values that make up that enum), and then a list of enums. So it's
//  a three level hierarchy.
//

namespace CQSL { namespace CQEnum {

//
//  The three types of enumerations we support. This controls what
//  operations are generated.
//
enum class EEnumTypes
{
    Standard
    , Bitmap
    , Arbitary

    , Count
};


// Represents a single enumeration value
class EnumValInfo
{
    public :
        EnumValInfo() = default;
        EnumValInfo(EnumValInfo&& evaliSrc) = default;
        ~EnumValInfo() = default;

        // Unimplemented
        EnumValInfo(const EnumValInfo&) = delete;
        EnumValInfo& operator=(const EnumValInfo&) = delete;
        EnumValInfo& operator=(EnumValInfo&&) = delete;

        void ParseFrom
        (
                    InputSrc&       srcFile
            , const bool            bOrdinal
            , const bool            bAltVal
            , const bool            bText1
            , const bool            bText2
            , const std::string&    strEnumName
        );


        int32_t         m_iAltValue = 0;
        int32_t         m_iOrdinal = 0;
        std::string     m_strName;
        std::string     m_strText1;
        std::string     m_strText2;
};


// Represents an overall enumeration definition
class EnumInfo
{
    public :
        //
        //  Used for synonyms, which have a name and ordinal value (which we get
        //  during parsing by looking up the other regular enum value that the
        //  synonym references and using his ordinal value.)
        //
        using KVSPair = std::pair<std::string, int32_t>;

        EnumInfo() noexcept = default;
        ~EnumInfo() noexcept = default;
        EnumInfo(EnumInfo&& enumiSrc) = default;

        // Unimplemented
        EnumInfo(const EnumInfo&) = delete;
        EnumInfo& operator=(const EnumInfo&) = delete;
        EnumInfo& operator=(EnumInfo&&) = delete;

        // Non-contiguous ones must provide an explicit ordinal value
        bool bExplicitOrdinal() const noexcept
        {
            return (m_eType == EEnumTypes::Arbitary) || (m_eType == EEnumTypes::Bitmap);
        }

        void ParseFrom(InputSrc& srcFile);

        // Find one of our values by name
        std::vector<EnumValInfo>::iterator itFindValue
        (
            const   std::string&        strToFind
        );

        // Find one of our values by its ordinal value
        std::vector<EnumValInfo>::iterator itFindValueByOrd
        (
            const   int32_t             iToFind
        );

        // Find one of our synonym by name
        std::vector<KVSPair>::iterator itFindSynonym
        (
            const   std::string&        strToFind
        );


        //
        //  We have flags to indicate what stuff the definition indicated
        //  should be generated for this enum, then the actual enumeration
        //  info we'll use to generate that output.
        //
        //  The monotonic flag is set if the ordinals are monotonic, which
        //  may be true even if it's not a standard type enum (they may just
        //  not start at zero which the standard contiguous type requires.)
        //  We can generate inc/dec commands for non-standard ones as long as
        //  they have monotonic ordinals.
        //
        bool                        m_bIsMonotonic = false;
        bool                        m_bText1 = false;
        bool                        m_bText2 = false;
        bool                        m_bDoAltVal = false;
        bool                        m_bDoInc = false;
        EEnumTypes                  m_eType = EEnumTypes::Count;
        std::string                 m_strName;
        std::string                 m_strUnderType;
        std::vector<KVSPair>        m_vSynonyms;
        std::vector<EnumValInfo>    m_vValues;
};


// Represents the overall enumerations definition block
class EnumInfoList
{
    public :
        EnumInfoList() noexcept = default;
        ~EnumInfoList() noexcept = default;

        /// Unimplemented
        EnumInfoList(const EnumInfoList&) = delete;
        EnumInfoList(EnumInfoList&&) = delete;
        EnumInfoList& operator=(const EnumInfoList&) = delete;
        EnumInfoList& operator=(EnumInfoList&&) = delete;

        // Parse our enums out and store them
        void ParseFrom(InputSrc& srcFile);

        // We track a list of defined enumerations
        std::vector<EnumInfo>       m_vEnumList;
};

}};


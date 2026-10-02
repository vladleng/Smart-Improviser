#include "core/analysis/LibraryArchive.h"
#include <array>
#include <algorithm>
#include <bit>
#include <limits>
#include <stdexcept>
#include <type_traits>

namespace smartimproviser::harmony
{
namespace
{
struct ArchiveError : std::runtime_error { using std::runtime_error::runtime_error; };
bool utf8(const std::string& s)
{
    for (std::size_t i=0;i<s.size();)
    {
        const auto c=static_cast<unsigned char>(s[i++]);
        if (c<128) { if (!c) return false; continue; }
        int count=0; std::uint32_t cp=0,minimum=0;
        if ((c&0xe0)==0xc0) { count=1; cp=c&31; minimum=128; }
        else if ((c&0xf0)==0xe0) { count=2; cp=c&15; minimum=2048; }
        else if ((c&0xf8)==0xf0) { count=3; cp=c&7; minimum=65536; }
        else return false;
        if (i+count>s.size()) return false;
        for (int n=0;n<count;++n) { const auto b=static_cast<unsigned char>(s[i++]); if ((b&0xc0)!=0x80) return false; cp=(cp<<6)|(b&63); }
        if (cp<minimum || cp>0x10ffff || (cp>=0xd800 && cp<=0xdfff)) return false;
    }
    return true;
}
struct Writer
{
    static constexpr bool reading=false;
    std::vector<std::uint8_t> bytes;
    void raw(std::uint64_t value, int size)
    {
        if (bytes.size()+size>maxLibraryArchiveBytes) throw ArchiveError("Archive exceeds size bound.");
        for (int n=0;n<size;++n) bytes.push_back(static_cast<std::uint8_t>(value>>(8*n)));
    }
    void text(std::string& value)
    {
        if (value.size()>65536 || !utf8(value)) throw ArchiveError("Invalid or oversized UTF-8 string.");
        raw(value.size(),4);
        if (bytes.size()+value.size()>maxLibraryArchiveBytes) throw ArchiveError("Archive exceeds size bound.");
        bytes.insert(bytes.end(),value.begin(),value.end());
    }
};
struct Reader
{
    static constexpr bool reading=true;
    std::span<const std::uint8_t> bytes; std::size_t offset=0;
    std::uint64_t raw(int size)
    {
        if (offset+size>bytes.size()) throw ArchiveError("Truncated archive.");
        std::uint64_t out=0; for (int n=0;n<size;++n) out|=std::uint64_t(bytes[offset++])<<(8*n);
        return out;
    }
    void text(std::string& value)
    {
        const auto size=raw(4);
        if (size>65536 || size>bytes.size()-offset) throw ArchiveError("Invalid string extent.");
        value.assign(reinterpret_cast<const char*>(bytes.data()+offset),static_cast<std::size_t>(size)); offset+=size;
        if (!utf8(value)) throw ArchiveError("Invalid UTF-8 text.");
    }
};
template<class T> struct Vector : std::false_type {};
template<class T> struct Vector<std::vector<T>> : std::true_type {};
template<class T> struct Optional : std::false_type {};
template<class T> struct Optional<std::optional<T>> : std::true_type {};
template<class A,class T> void field(A& a,T& v)
{
    if constexpr (std::is_same_v<T,bool>)
    {
        if constexpr (A::reading) { const auto b=a.raw(1); if (b>1) throw ArchiveError("Invalid boolean."); v=b!=0; }
        else a.raw(v?1:0,1);
    }
    else if constexpr (std::is_enum_v<T>)
    {
        auto value=static_cast<std::underlying_type_t<T>>(v); field(a,value);
        if constexpr (A::reading) v=static_cast<T>(value);
    }
    else if constexpr (std::is_integral_v<T>)
    {
        using U=std::make_unsigned_t<T>;
        if constexpr (A::reading) { const auto u=static_cast<U>(a.raw(sizeof(T))); v=std::bit_cast<T>(u); }
        else a.raw(static_cast<U>(v),sizeof(T));
    }
    else if constexpr (std::is_same_v<T,double>)
    {
        static_assert(sizeof(double)==8 && std::numeric_limits<double>::is_iec559);
        if constexpr (A::reading) v=std::bit_cast<double>(a.raw(8));
        else a.raw(std::bit_cast<std::uint64_t>(v),8);
    }
    else if constexpr (std::is_same_v<T,std::string>) a.text(v);
    else if constexpr (Vector<T>::value)
    {
        std::uint32_t count=0;
        if constexpr (!A::reading) { if (v.size()>65536) throw ArchiveError("Too many array entries."); count=static_cast<std::uint32_t>(v.size()); }
        field(a,count);
        if (count>65536) throw ArchiveError("Too many array entries.");
        if constexpr (A::reading) { if (count>a.bytes.size()-a.offset) throw ArchiveError("Truncated array."); v.resize(count); }
        for (auto& item:v) field(a,item);
    }
    else if constexpr (Optional<T>::value)
    {
        bool present=v.has_value(); field(a,present);
        if constexpr (A::reading) { if (present) v.emplace(); else v.reset(); }
        if (present) field(a,*v);
    }
    else if constexpr (std::is_same_v<T,RelativePitch>) { field(a,v.chordIndex); field(a,v.degree); field(a,v.chromaticOffset); }
    else if constexpr (std::is_same_v<T,PhraseFingering>) { field(a,v.stringNumber); field(a,v.fret); }
    else if constexpr (std::is_same_v<T,PhraseRegisterReference>) { field(a,v.chordIndex); field(a,v.rootMidiNote); }
    else if constexpr (std::is_same_v<T,PhraseNote>)
    { field(a,v.pitch); field(a,v.beatOffset); field(a,v.durationBeats); field(a,v.target); field(a,v.harmonicRole); field(a,v.octaveOffset); field(a,v.fingering); }
    else if constexpr (std::is_same_v<T,AnalysisEvidence>)
    {
        field(a,v.confidence); field(a,v.interpretation); field(a,v.alternativeCount); field(a,v.flags);
        if (v.confidence>ConfidenceLevel::confirmed || v.interpretation>InterpretationStatus::ambiguous || (v.flags&~std::uint32_t(0x1ffff)))
            throw ArchiveError("Unknown evidence metadata.");
    }
    else if constexpr (std::is_same_v<T,PhraseSlotRequirement>)
    { field(a,v.chordIndex); field(a,v.sourceRuleId); field(a,v.sourceRuleVersion); field(a,v.sourceRootOffset); field(a,v.chordQuality); field(a,v.destinationQuality); field(a,v.confirmedDestinationRequired); field(a,v.harmonicPattern); }
    else if constexpr (std::is_same_v<T,PhraseApproach>) { field(a,v.chordIndex); field(a,v.conceptRuleId); std::vector<std::uint64_t> indices;
        if constexpr (!A::reading) indices.assign(v.noteIndices.begin(),v.noteIndices.end());
        field(a,indices);
        if constexpr (A::reading) { v.noteIndices.clear(); for (auto index:indices) { if (index>std::numeric_limits<std::size_t>::max()) throw ArchiveError("Index is not representable."); v.noteIndices.push_back(static_cast<std::size_t>(index)); } }
    }
    else if constexpr (std::is_same_v<T,TensionSpan>) { field(a,v.startBeat); field(a,v.endBeat); field(a,v.level); }
    else if constexpr (std::is_same_v<T,PhraseTensionProfile>) { field(a,v.defined); field(a,v.spans); }
    else if constexpr (std::is_same_v<T,Phrase>)
    {
        field(a,v.id); field(a,v.name); field(a,v.harmonicPattern); field(a,v.tensionLevel); field(a,v.role); field(a,v.startDegree); field(a,v.targetDegree);
        field(a,v.notes); field(a,v.evidence); field(a,v.tensionClassified); field(a,v.harmonicRequirements); field(a,v.approaches);
        field(a,v.tensionProfile); field(a,v.registerReferences); field(a,v.conceptRuleIds);
    }
    else if constexpr (std::is_same_v<T,LibraryItemReference>) { field(a,v.domain); field(a,v.id); field(a,v.revision); }
    else if constexpr (std::is_same_v<T,LibraryLineage>) { field(a,v.kind); field(a,v.parent); field(a,v.ancestors); }
    else if constexpr (std::is_same_v<T,LibrarySourceMetadata>) { field(a,v.source); field(a,v.author); field(a,v.license); field(a,v.permissionEvidence); field(a,v.permission); }
    else if constexpr (std::is_same_v<T,IdeaNote>)
    { field(a,v.pitch); field(a,v.beatOffset); field(a,v.durationBeats); field(a,v.octaveOffset); field(a,v.harmonicRole); field(a,v.target); field(a,v.fingering); }
    else if constexpr (std::is_same_v<T,Idea>)
    { field(a,v.text); field(a,v.conceptRuleIds); field(a,v.rhythmNotes); field(a,v.harmonyNotes); field(a,v.notes); field(a,v.registerReferences); field(a,v.harmonicPattern); }
    else if constexpr (std::is_same_v<T,LibraryRecord>)
    {
        field(a,v.id); field(a,v.revision); field(a,v.domain); field(a,v.name); field(a,v.tags); field(a,v.explanation); field(a,v.source); field(a,v.lineage);
        std::uint8_t kind=std::holds_alternative<Idea>(v.content)?0:1; field(a,kind);
        if (kind>1) throw ArchiveError("Unknown content type.");
        if constexpr (A::reading) { if (kind==0) v.content=Idea{}; else v.content=Phrase{}; }
        if (kind==0) field(a,std::get<Idea>(v.content)); else field(a,std::get<Phrase>(v.content));
    }
    else if constexpr (std::is_same_v<T,LibraryUserSnapshot>) { field(a,v.records); field(a,v.reservedIds); }
    else if constexpr (std::is_same_v<T,LibraryArchive>) { field(a,v.storeId); field(a,v.generation); field(a,v.users); }
    else static_assert(sizeof(T)==0,"Unsupported archive field.");
}
std::uint32_t crc(std::span<const std::uint8_t> bytes)
{
    std::uint32_t out=0xffffffff;
    for (auto b:bytes) { out^=b; for(int bit=0;bit<8;++bit) out=(out>>1)^(0xedb88320u & (0u-(out&1u))); }
    return ~out;
}
constexpr std::array<std::uint8_t,8> magic={'S','I','L','I','B','0','1',0};
LibraryArchiveResult error(LibraryArchiveStatus status,const char* text)
{ LibraryArchiveResult r; r.status=status; r.explanation=text; return r; }
bool valid(const LibraryArchive& archive)
{
    PhraseLibrary check;
    return archive.storeId.find_first_not_of(" \t\r\n")!=std::string::npos && archive.generation>0 && check.restoreUserSnapshot(archive.users).succeeded();
}
}
LibraryArchiveResult encodeLibraryArchive(const LibraryArchive& value)
{
    if (!valid(value)) return error(LibraryArchiveStatus::invalidSnapshot,"Invalid user snapshot or archive identity/generation.");
    try
    {
        auto copy=value; Writer payload; field(payload,copy);
        Writer envelope; envelope.bytes.insert(envelope.bytes.end(),magic.begin(),magic.end());
        envelope.raw(1,4); envelope.raw(payload.bytes.size(),4); envelope.raw(crc(payload.bytes),4);
        if (envelope.bytes.size()+payload.bytes.size()>maxLibraryArchiveBytes) return error(LibraryArchiveStatus::tooLarge,"Archive exceeds size bound.");
        envelope.bytes.insert(envelope.bytes.end(),payload.bytes.begin(),payload.bytes.end());
        LibraryArchiveResult out; out.bytes=std::move(envelope.bytes); return out;
    }
    catch (const ArchiveError& e) { auto r=error(LibraryArchiveStatus::invalidSnapshot,""); r.explanation=e.what(); return r; }
}
LibraryArchiveResult decodeLibraryArchive(std::span<const std::uint8_t> bytes)
{
    if (bytes.size()>maxLibraryArchiveBytes) return error(LibraryArchiveStatus::tooLarge,"Archive exceeds size bound.");
    if (bytes.size()<20 || !std::equal(magic.begin(),magic.end(),bytes.begin())) return error(LibraryArchiveStatus::malformed,"Unrecognized library header.");
    try
    {
        Reader header{bytes,8}; const auto schema=header.raw(4);
        if (schema>1) return error(LibraryArchiveStatus::futureSchema,"Library schema is newer; preserve the file.");
        if (schema!=1) return error(LibraryArchiveStatus::unsupportedSchema,"Unsupported library schema; preserve the file.");
        const auto size=header.raw(4), checksum=header.raw(4);
        if (size!=bytes.size()-20 || crc(bytes.subspan(20))!=checksum) return error(LibraryArchiveStatus::malformed,"Invalid archive length/checksum.");
        Reader payload{bytes.subspan(20)}; LibraryArchive value; field(payload,value);
        if (payload.offset!=payload.bytes.size()) return error(LibraryArchiveStatus::malformed,"Unexpected trailing data.");
        if (!valid(value)) return error(LibraryArchiveStatus::invalidSnapshot,"Decoded library data is invalid.");
        LibraryArchiveResult out; out.archive=std::move(value); return out;
    }
    catch (const ArchiveError& e) { auto r=error(LibraryArchiveStatus::malformed,""); r.explanation=e.what(); return r; }
}
LibraryOperation mergeLibraryUserSnapshot(LibraryUserSnapshot& target,const LibraryUserSnapshot& incoming)
{
    PhraseLibrary validator;
    auto check=validator.restoreUserSnapshot(target); if (!check.succeeded()) return check;
    check=validator.restoreUserSnapshot(incoming); if (!check.succeeded()) return check;
    std::set<std::string> ids(target.reservedIds.begin(),target.reservedIds.end());
    for (const auto& id:incoming.reservedIds) if (!ids.insert(id).second)
    { LibraryOperation r; r.status=LibraryStatus::revisionConflict; r.explanation="Imported identity already exists or is reserved, regardless of revision."; return r; }
    auto merged=target; merged.records.insert(merged.records.end(),incoming.records.begin(),incoming.records.end());
    merged.reservedIds.assign(ids.begin(),ids.end());
    check=validator.restoreUserSnapshot(merged); if (!check.succeeded()) return check;
    target=std::move(merged); return {};
}
}

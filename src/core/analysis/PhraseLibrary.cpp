#include "core/analysis/PhraseLibrary.h"
#include <limits>
#include <random>
#include <utility>

namespace smartimproviser::harmony
{
namespace
{
bool validDomain(LibraryDomain d) { return d == LibraryDomain::common || d == LibraryDomain::user; }
bool hasText(const std::string& s) { return s.find_first_not_of(" \t\r\n") != std::string::npos; }
LibraryOperation failure(LibraryStatus s, const char* why)
{
    LibraryOperation result; result.status=s; result.explanation=why; return result;
}
LibraryOperation validated(const LibraryRecord& r)
{
    LibraryOperation result;
    result.validation=validateLibraryRecord(r);
    if (!result.validation->structurallyValid)
    {
        result.status=LibraryStatus::invalidRecord;
        result.explanation="The library record is structurally invalid; no data changed.";
    }
    return result;
}
bool sameReference(const LibraryItemReference& a, const LibraryItemReference& b)
{
    return a.domain==b.domain && a.id==b.id && a.revision==b.revision;
}
bool sameLineage(const std::optional<LibraryLineage>& a, const std::optional<LibraryLineage>& b)
{
    if (a.has_value()!=b.has_value()) return false;
    if (!a) return true;
    if (a->kind!=b->kind || !sameReference(a->parent,b->parent) || a->ancestors.size()!=b->ancestors.size())
        return false;
    for (std::size_t i=0;i<a->ancestors.size();++i)
        if (!sameReference(a->ancestors[i],b->ancestors[i])) return false;
    return true;
}
}
std::string makeLibraryItemId()
{
    std::random_device random;
    constexpr char hex[]="0123456789abcdef";
    unsigned char bytes[16];
    for (auto& b:bytes) b=static_cast<unsigned char>(random());
    bytes[6]=static_cast<unsigned char>((bytes[6]&0x0f)|0x40);
    bytes[8]=static_cast<unsigned char>((bytes[8]&0x3f)|0x80);
    std::string id="user:";
    for (int i=0;i<16;++i)
    {
        if (i==4 || i==6 || i==8 || i==10) id+='-';
        id+=hex[bytes[i]>>4]; id+=hex[bytes[i]&0xf];
    }
    return id;
}
PhraseLibrary::PhraseLibrary(IdGenerator generator) : generateId_(std::move(generator)) {}
LibraryOperation PhraseLibrary::initializeCommonCatalog(const std::vector<LibraryRecord>& records)
{
    if (commonInitialized_) return failure(LibraryStatus::readOnly,"The common snapshot is already initialized.");
    std::map<std::string,LibraryRecord> next;
    auto reserved=reservedIds_;
    for (const auto& r:records)
    {
        if (r.domain!=LibraryDomain::common)
            return failure(LibraryStatus::invalidIdentity,"The common catalog accepts only common entries.");
        auto check=validated(r);
        if (!check.succeeded()) return check;
        if (!check.validation->distributable)
        {
            check.status=LibraryStatus::invalidRecord;
            check.explanation="Common content requires complete musical and distribution metadata.";
            return check;
        }
        if (!reserved.insert(r.id).second)
            return failure(LibraryStatus::invalidIdentity,"Duplicate or already reserved library identity.");
        next.emplace(r.id,r);
    }
    common_.swap(next); reservedIds_.swap(reserved); commonInitialized_=true;
    return {};
}
std::optional<LibraryRecord> PhraseLibrary::get(LibraryDomain domain, const std::string& id) const
{
    if (!validDomain(domain)) return std::nullopt;
    const auto& entries=domain==LibraryDomain::common ? common_ : user_;
    const auto it=entries.find(id);
    if (it==entries.end()) return std::nullopt;
    return it->second;
}
std::vector<LibraryRecord> PhraseLibrary::list(LibraryDomain domain) const
{
    std::vector<LibraryRecord> out;
    if (!validDomain(domain)) return out;
    const auto& entries=domain==LibraryDomain::common ? common_ : user_;
    for (const auto& [id,r]:entries) out.push_back(r);
    return out; // Deterministic ID order, independent values.
}
LibraryOperation PhraseLibrary::insertNew(LibraryRecord record)
{
    if (!generateId_) return failure(LibraryStatus::idGenerationFailed,"No item ID generator.");
    std::string id;
    try
    {
        for (int attempt=0;attempt<8;++attempt)
        {
            auto candidate=generateId_();
            if (hasText(candidate) && !reservedIds_.contains(candidate))
            { id=std::move(candidate); break; }
        }
    }
    catch (...) { return failure(LibraryStatus::idGenerationFailed,"ID generation failed; no data changed."); }
    if (id.empty()) return failure(LibraryStatus::idGenerationFailed,"No unused identity generated after eight attempts.");
    record.id=std::move(id); record.revision=1; record.domain=LibraryDomain::user;
    auto check=validated(record);
    if (!check.succeeded()) return check;
    // Fully prepare values before exposing them in the store.
    check.record=record;
    auto allReserved=reservedIds_, userReserved=userReservedIds_;
    allReserved.insert(record.id); userReserved.insert(record.id);
    if (record.lineage)
    {
        auto reserveAncestor=[&](const LibraryItemReference& reference)
        {
            if (reference.domain!=LibraryDomain::user) return true;
            if (common_.contains(reference.id)) return false;
            allReserved.insert(reference.id); userReserved.insert(reference.id); return true;
        };
        if (!reserveAncestor(record.lineage->parent))
            return failure(LibraryStatus::invalidIdentity,"User ancestry collides with a common identity.");
        for (const auto& ancestor:record.lineage->ancestors)
            if (!reserveAncestor(ancestor))
                return failure(LibraryStatus::invalidIdentity,"User ancestry collides with a common identity.");
    }
    if (userReserved.size()>65536) return failure(LibraryStatus::invalidRecord,"Too many reserved user identities.");
    const auto itemId=record.id;
    auto [position,inserted]=user_.emplace(itemId,std::move(record));
    if (!inserted) return failure(LibraryStatus::invalidIdentity,"Identity already exists.");
    reservedIds_.swap(allReserved); userReservedIds_.swap(userReserved);
    return check;
}
LibraryUserSnapshot PhraseLibrary::userSnapshot() const
{
    LibraryUserSnapshot snapshot;
    for (const auto& [id,record]:user_) snapshot.records.push_back(record);
    snapshot.reservedIds.assign(userReservedIds_.begin(),userReservedIds_.end());
    return snapshot;
}
LibraryOperation PhraseLibrary::restoreUserSnapshot(const LibraryUserSnapshot& snapshot)
{
    if (snapshot.records.size()>10000 || snapshot.reservedIds.size()>65536)
        return failure(LibraryStatus::invalidRecord,"Library snapshot exceeds supported bounds.");
    std::map<std::string,LibraryRecord> users;
    std::set<std::string> userReserved, allReserved;
    for (const auto& [id,record]:common_) allReserved.insert(id);
    for (const auto& id:snapshot.reservedIds)
        if (!hasText(id) || !userReserved.insert(id).second || !allReserved.insert(id).second)
            return failure(LibraryStatus::invalidIdentity,"Invalid, duplicate or common-colliding user reservation.");
    for (const auto& record:snapshot.records)
    {
        if (record.domain!=LibraryDomain::user || !userReserved.contains(record.id))
            return failure(LibraryStatus::invalidIdentity,"Loaded users need their own reserved identities.");
        auto check=validated(record);
        if (!check.succeeded()) return check;
        if (record.lineage)
        {
            const auto reservedAncestor=[&](const LibraryItemReference& r)
            { return r.domain!=LibraryDomain::user || userReserved.contains(r.id); };
            if (!reservedAncestor(record.lineage->parent))
                return failure(LibraryStatus::invalidIdentity,"User ancestry identity must remain reserved.");
            for (const auto& ancestor:record.lineage->ancestors)
                if (!reservedAncestor(ancestor))
                    return failure(LibraryStatus::invalidIdentity,"User ancestry identity must remain reserved.");
        }
        if (!users.emplace(record.id,record).second)
            return failure(LibraryStatus::invalidIdentity,"Duplicate loaded user identity.");
    }
    user_.swap(users); userReservedIds_.swap(userReserved); reservedIds_.swap(allReserved);
    return {};
}
LibraryOperation PhraseLibrary::addUser(const LibraryRecord& draft)
{
    if (draft.domain!=LibraryDomain::user)
        return failure(draft.domain==LibraryDomain::common ? LibraryStatus::readOnly : LibraryStatus::invalidIdentity,
            "User insertion requires the user domain.");
    if (!draft.id.empty() || draft.revision!=0)
        return failure(LibraryStatus::invalidIdentity,"New records must not bring an existing identity/revision.");
    if (draft.lineage)
        return failure(LibraryStatus::lineageConflict,"New original content cannot claim a copy lineage; use copy/variant.");
    return insertNew(draft);
}
LibraryOperation PhraseLibrary::updateUser(const LibraryRecord& incoming, std::uint64_t expectedRevision)
{
    if (incoming.domain!=LibraryDomain::user)
        return failure(incoming.domain==LibraryDomain::common ? LibraryStatus::readOnly : LibraryStatus::invalidIdentity,
            "Common records cannot be edited through the user API.");
    const auto it=user_.find(incoming.id);
    if (it==user_.end())
        return failure(common_.contains(incoming.id) ? LibraryStatus::readOnly : LibraryStatus::notFound,"No editable user record with this identity.");
    if (expectedRevision==0 || it->second.revision!=expectedRevision || incoming.revision!=expectedRevision)
        return failure(LibraryStatus::revisionConflict,"Stored and submitted revisions must equal the expected revision.");
    if (!sameLineage(incoming.lineage,it->second.lineage))
        return failure(LibraryStatus::lineageConflict,"Editing cannot rewrite recorded source provenance.");
    if (expectedRevision==std::numeric_limits<std::uint64_t>::max())
        return failure(LibraryStatus::revisionOverflow,"Revision cannot be incremented.");
    auto next=incoming; ++next.revision;
    auto check=validated(next);
    if (!check.succeeded()) return check;
    check.record=next;
    using std::swap; swap(it->second,next);
    return check;
}
LibraryOperation PhraseLibrary::eraseUser(const std::string& id, std::uint64_t expectedRevision)
{
    const auto it=user_.find(id);
    if (it==user_.end())
        return failure(common_.contains(id) ? LibraryStatus::readOnly : LibraryStatus::notFound,"No editable user record with this identity.");
    if (expectedRevision==0 || it->second.revision!=expectedRevision)
        return failure(LibraryStatus::revisionConflict,"Deletion requires the exact current revision.");
    user_.erase(it); // Keep the identity reserved; copies own their material.
    return {};
}
LibraryOperation PhraseLibrary::derive(const LibraryItemReference& reference, LibraryDerivation kind)
{
    if (!validDomain(reference.domain) || !hasText(reference.id) || reference.revision==0)
        return failure(LibraryStatus::invalidIdentity,"A source needs an explicit domain, ID and exact revision.");
    auto source=get(reference.domain,reference.id);
    if (!source) return failure(LibraryStatus::notFound,"The source entry is unavailable.");
    if (source->revision!=reference.revision)
        return failure(LibraryStatus::revisionConflict,"The source has changed; reselect its current revision.");
    LibraryLineage line; line.kind=kind; line.parent=reference;
    if (source->lineage)
    {
        line.ancestors.push_back(source->lineage->parent);
        line.ancestors.insert(line.ancestors.end(),source->lineage->ancestors.begin(),source->lineage->ancestors.end());
    }
    source->id.clear(); source->revision=0;
    source->lineage=std::move(line);
    return insertNew(std::move(*source));
}
LibraryOperation PhraseLibrary::copyToUser(const LibraryItemReference& r) { return derive(r,LibraryDerivation::copy); }
LibraryOperation PhraseLibrary::createVariant(const LibraryItemReference& r) { return derive(r,LibraryDerivation::variant); }
}

#pragma once
#include "core/analysis/LibraryValidation.h"
#include <functional>
#include <map>
#include <set>

namespace smartimproviser::harmony
{
enum class LibraryStatus : std::uint8_t
{
    success, invalidRecord, invalidIdentity, readOnly, notFound,
    revisionConflict, idGenerationFailed, lineageConflict, revisionOverflow
};
struct LibraryOperation
{
    LibraryStatus status = LibraryStatus::success;
    std::optional<LibraryRecord> record; // Detached value, never a mutable storage handle.
    std::optional<LibraryValidation> validation;
    std::string explanation;
    bool succeeded() const noexcept { return status == LibraryStatus::success; }
};
std::string makeLibraryItemId(); // Host-neutral random UUID; library still checks collisions.

// In-memory, single-owner API. No disk I/O or cross-instance/process locking.
// The 0.5c backend will coordinate durable snapshots/revisions.
class PhraseLibrary
{
public:
    using IdGenerator = std::function<std::string()>;
    explicit PhraseLibrary(IdGenerator generator = makeLibraryItemId);

    // Bootstrap an immutable common snapshot once. Atomic validation; later
    // catalog releases use a new snapshot, never this user mutation API.
    LibraryOperation initializeCommonCatalog(const std::vector<LibraryRecord>&);

    std::optional<LibraryRecord> get(LibraryDomain, const std::string& id) const;
    std::vector<LibraryRecord> list(LibraryDomain) const;
    LibraryOperation addUser(const LibraryRecord& draft); // Empty id, revision 0.
    LibraryOperation updateUser(const LibraryRecord&, std::uint64_t expectedRevision);
    LibraryOperation eraseUser(const std::string& id, std::uint64_t expectedRevision);
    LibraryOperation copyToUser(const LibraryItemReference&);
    LibraryOperation createVariant(const LibraryItemReference&);
private:
    LibraryOperation derive(const LibraryItemReference&, LibraryDerivation);
    LibraryOperation insertNew(LibraryRecord);
    IdGenerator generateId_;
    std::map<std::string,LibraryRecord> common_, user_;
    std::set<std::string> reservedIds_; // Includes removed identities; no reuse in this store.
    bool commonInitialized_ = false;
};
}

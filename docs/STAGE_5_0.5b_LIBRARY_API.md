# 0.5b — In-memory Library API / independent copies

0.5a accepted by Vlad on 2026-10-01; PR #59 merged in main (37d0b85).
0.5b implementation pending Windows CI and Studio Pro regression acceptance.
Stable remains 0.5. Persistence belongs to 0.5c and library UI to 0.5d.

## Ownership and bootstrap

PhraseLibrary is a host-neutral, single-owner in-memory store. It owns detached
values in common/user domains and returns detached snapshots from get/list and
successful operations. Caller edits cannot bypass revision checking.

initializeCommonCatalog validates the entire common vector, including the
0.5a distribution requirements and duplicate identities, before installing it.
Failed bootstrap changes no entries and reserves no IDs; it can be retried.
A successfully installed common catalog is immutable for the store lifetime.
Catalog version refresh is deliberately outside this user mutation API: future
loading/persistence will create a new immutable common snapshot. There is no
common-content edit, deletion or implicit upsert in this checkpoint.
User copies do not depend on continued availability of the catalog instance.

## User operations

- addUser: new original content with empty ID/revision 0, user domain and no
  fabricated lineage. The API assigns a new ID/revision 1. Draft Ideas need only
  structurally valid supplied fields; missing tagging/tension is not invented.
- get/list: detached snapshots, separated by domain; list order is deterministic
  by identity, not a claim of musical relevance.
- updateUser: exact expected revision must match stored and submitted revisions.
  Valid content replaces the value and increments its revision. Source lineage
  is immutable; edits cannot relabel their provenance.
- eraseUser: exact current revision is required; common identity is read-only.
  Removed IDs remain reserved for this in-memory store, so they cannot later
  identify unrelated material referenced by an older copy.
- copyToUser/createVariant: require domain/ID/exact current source revision.
  Assign a fresh user ID/revision 1 and record derivation kind, exact parent
  revision and ordered older ancestors. Payload/source metadata remain value
  snapshots. Editing, removing or changing a user source cannot rewrite copies
  or variants, which remain editable with unavailable parents.

The default ID generator emits namespaced random UUID v4 values; an injectable
generator supports deterministic tests and future integration. Eight attempts
allow live/deleted-ID collision checks across both domains. Missing/throwing/
blank/colliding generators return an explicit error without partial mutation.
UUIDs alone do not replace future persistence conflict/locking checks.

LibraryOperation distinguishes success, invalid record/identity, read-only,
not found, stale revision, ID generation failure, lineage conflict and revision
overflow. Validation details explain structural errors. Rejected operations
leave existing entries and revisions intact. Insert is not import/upsert;
portable IDs/revisions and durable deletion reservations are a 0.5c contract.

## Scope and verification

No disk I/O, multi-process lock, user-library synchronization, search ranking,
library UI or actual song placement is claimed here. The 0.5c backend must
coordinate loaded versions, retained identities, concurrent writers and durable
snapshots; the in-memory object is used sequentially by one owner.

SmartImproviserPhraseLibraryTests covers atomic bootstrap, common read-only
protection, detached reads/outputs, original insertion, revision conflicts,
invalid-edit rollback, independent copies of full Phrase and partial Idea,
multi-generation variant lineage, immutable provenance, source edit/deletion,
generator collisions/failures and UUID shape. Existing 28 regressions retained;
the full Windows plugin build should run 29 tests.

After green CI, Studio Pro regression:
1. Install Smart-Improviser-0.5b-Windows and confirm internal version 0.5b.
2. Open the usual project; seek/PLAY updates chord/turn/context and note viewers.
3. Check the white current-chord root and All/T1/T2/T3 filters.
4. Verify existing manual tensions after reopen and in another project.
5. Accept this checkpoint before 0.5c. The API itself is tested automatically;
   no new library UI is available to inspect before 0.5d.

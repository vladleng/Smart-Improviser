# 0.5c — User library persistence / portable archive

0.5b accepted by Vlad on 2026-10-01; PR #60 merged in main (2c3591c).
0.5c implemented on its own branch, pending Windows CI and Studio Pro regression.
Stable remains 0.5; library UI remains 0.5d.

## Backend decision before implementation

Personal content is shared across projects in a separate local user file:
userApplicationDataDirectory / Moon River Studio / Smart Improviser / user-library.silibrary.
The common catalog is not serialized into this file. Existing manual-tensions.xml
remains the independent Stage 4 preference profile. No Song workspace data or
song/project identity is invented here. Plugin construction loads a valid user
snapshot; malformed/unrecognized data leaves current memory unchanged.
No file work runs in processBlock or gets serialized into DAW plugin state.
Construction does not create or overwrite an empty library file. Writing starts
only when the library persistence API receives an explicit commit/import.

## Format: portable binary schema 1

LibraryArchive.h/cpp is host-neutral. Header: 8 magic bytes SILIB01 plus NUL,
little-endian u32 schema, u32 payload length, u32 CRC32 of payload. Schema 1 has
a field-ordered payload: store ID, u64 generation and the user snapshot.
All integral fields use their fixed model width, approach indices use u64,
double uses IEEE-754 binary64, optional presence and booleans use strict 0/1,
UTF-8 strings and arrays use u32 lengths. Header/trailing bytes, extent/checksum,
UTF-8, field bounds and model structure are checked before loading or writing.
Adding/reordering serialized fields requires a schema change and explicit migration.

Limits: 32 MiB/file, 64 KiB/string, 65536 elements/array, 10000 live records,
65536 reserved user identities. NUL text is rejected, and unsupported schema
requires an explicit later migration; a future schema is never silently opened
as empty or rewritten. CRC detects accidental corruption, not authenticity.

Full Phrase/Idea payload, note timing/register/roles/targets, source applications,
source-rule versions, approaches, concepts, tension profile, evidence, tags,
author/source/license and exact lineage are retained. Missing optional draft
data stays unknown; no T1 or MIDI height is invented. Deleted user identities
remain durable reservations. Common references in lineage may be unavailable:
owned copied material is sufficient to reopen independently.

## File commit and multiple instances/processes

Each operation takes a static thread lock and a named inter-process lock derived
from the full normalized path. The disk archive is reread while holding the lock.
Commit requires the expected file-existence/store-ID/generation/checksum stamp.
An intervening save, deletion or replacement produces an explicit conflict; the
caller must reload/reconcile before trying again. No silent last-writer-wins merge.
Existing malformed/future-schema files block both commit and import.

A new generation is validated/encoded before writing a unique temporary file in
the same directory. Native write/flush and reread byte verification must succeed.
Windows replaces through MoveFileExW with REPLACE_EXISTING and WRITE_THROUGH;
the POSIX fallback uses rename. No separate delete of the old file occurs.
Any failure before replacement leaves its bytes intact; failed commit does not
mutate the caller's in-memory edits. Process death releases the OS lock; an
orphan temporary file is not mistaken for the active library. This does not
promise cloud/network filesystem synchronization or recovery from every hardware
failure; POSIX durability beyond rename is not verified by Windows CI.

## Export/import

exportData returns a portable encoded archive without changing the active file.
importData validates the whole input, then rereads/merges/writes under the lock.
Imported item IDs/revisions, payload and lineage are retained. Every overlapping
ID/reservation, including a tombstone or an equal ID/revision, is an explicit
conflict and the whole batch fails unchanged. There is no implicit upsert,
revision overwrite or deduplication that drops provenance. UI conflict-resolution
and portable file selection belong to the upcoming library UI.

PhraseLibrary gains an atomic backend restore and a user snapshot containing
live records plus deleted-ID reservations. A bad restore cannot replace current
memory or bypass common-identity protection. If an already populated in-memory
library's file disappears, loadInto refuses to clear that memory silently.

## Verification and Studio Pro gate

LibraryArchiveTests verifies every semantic field and unknown optional values,
UTF-8, exact roundtrip, unavailable-source restore, malformed/future/truncated
archives, strict import conflicts and persistent deleted-ID reservations.
LibraryStorageTests uses isolated real files and checks fresh-instance reopen,
two project-like instances, stale writer conflicts, failure injection before
write/replacement, preservation of corrupt/future target files, export/import,
source deletion and retained variants. A subprocess test starts two separate OS
processes that both read one generation before either commits: exactly one
succeeds, the other reports conflict; the winner's file remains valid.
Full plugin build should run 31 CTest targets (29 prior plus these two).

After green Windows CI:
1. Install Smart-Improviser-0.5c-Windows and confirm 0.5c.
2. Open/reopen usual projects; context, note viewers, white chord root and
   All/T1/T2/T3 filters work.
3. Confirm existing manual labels across projects and DAW restart.
4. Report regression or accept 0.5c before moving to 0.5d.

Library creation/editing/export buttons are not yet exposed before 0.5d, so actual
library disk behavior is checked through automated real-file/subprocess tests,
not misrepresented as a completed manual UI test.

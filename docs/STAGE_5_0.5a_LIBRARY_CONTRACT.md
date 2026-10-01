# 0.5a — Library record / partial Idea contract

Accepted by Vlad on 2026-10-01 after Studio Pro regression. PR #59 merged in main (37d0b85).
Windows Build #425 passed: 28/28 CTest and Smart-Improviser-0.5a-Windows artifact.
Base: stable 0.5, main c03d7f6. Stable remains 0.5; this checkpoint is not Stage 5 completion.

## Data ownership and identity

LibraryRecord owns exactly one content value: Idea or the existing Phrase.
Outer id/revision/domain identify the library entry. Phrase id/name are retained
for backward compatibility as musical payload metadata; they do not identify a
library copy. New IDs, revision conflict handling and copy/variant operations
belong to 0.5b, not this value-model checkpoint.

Lineage records the immediate parent domain/ID/exact revision, derivation
(copy or variant), and earlier ancestors. A derived item must have another ID.
Missing revisions and repeated/self ancestry are rejected. Value copies own
their notes, sources, approaches, profile, register, concepts and fingering:
changes to a copied value cannot alter the source. There is no automatic link
that rewrites a copy when a source changes or disappears.

Source/author/license/permission evidence are independent from harmonic
source-rule IDs/versions. Permission is explicit (unknown, ownWork, publicDomain,
licensed); a URL or license string alone does not constitute checked permission.
Metadata declarations still need human verification before distributing content.

## Partial content

Idea stores text, concepts, rhythm/harmony notes and optional partial note fields.
Absent pitch/timing/register/role/target values remain unknown. A present pitch
requires a known degree/accidental; chordIndex -1 can preserve an unknown slot.
A present duration must be positive, and supplied times finite. No title or
number of filled fields automatically converts an Idea into a ready Phrase.

## Explicit register and contour

PhraseNote adds optional octaveOffset and optional string/fret reference at the
end, preserving previous aggregate initializers. Phrase adds registerReferences
and conceptRuleIds; source requirements/approaches/roles/targets/tension remain
in the accepted Phrase model, without a second copy in LibraryRecord.

Each register reference gives a harmonic slot's written example root in MIDI
numbering (C4 = 60). It is an example, not a DAW root or a new inferred key.
For degree d, using major-scale intervals 0,2,4,5,7,9,11:

    example pitch = root MIDI + interval[(d-1)%7] + 12*((d-1)/7)
                    + accidental + 12*octaveOffset

Compound degrees carry their written octave. Offset 0 is explicit; null is
unknown. Root references plus offsets preserve contour across different chord
roots without storing another sequence of note pitches. The helper returns no
pitch for missing/ambiguous register, invalid degree or MIDI overflow.
Legacy pitch-class phrases remain supported by assessPhrase and are not given
invented octaves. Fingering is optional reference only; no tuning, feasible
position or fingering is generated.

## Three independent checks

- Structure: identities, enums, finite ordered timing, degrees, optional
  register/fingering, source bindings, approach indices, lineage and existing
  descriptive tension-profile validation.
- Search readiness: a structurally valid Phrase with notes, explicit example
  register, versioned sources for source-based notes and approach grouping.
  Outside policy is unsupported. Missing tension classification is allowed.
  This is completeness, NOT a promise of harmonic compatibility. Every candidate
  still needs assessPhrase with actual context; unknown catalog versions are
  revalidated there. Actual next chord and assumed destination remain separate.
- Distribution: ready common content with title, explanation, author/source,
  license and explicit permission evidence. Private drafts need none of this.

LibraryValidation never changes material, assigns T1 or writes a profile.
Existing shared user tensions are not copied into every library record.

## Verification

New SmartImproviserLibraryContractTests covers partial content, register and
compound degrees, missing and ambiguous roots, numerical failures, source versions,
approaches, fingering, existing tension validation, nested value copies, exact
lineage, publication requirements and harmonic-first matcher compatibility.
Existing 27 Windows tests are retained; the full plugin build should run 28 tests.

## Studio Pro regression checklist after green Windows CI

1. Install Smart-Improviser-0.5a-Windows; package name remains Smart Improviser.vst3.
2. Confirm 0.5a in the plugin.
3. Open an existing project: chord/pattern/material context, notation and fretboard
   update normally on seek/PLAY.
4. Confirm the white root is the current chord root.
5. Check All/T1/T2/T3 and existing manual labels; reopen and another project retain them.
6. Report any regression; if these pass, accept 0.5a before proceeding to 0.5b.

There is no library UI, disk persistence, mass shared vocabulary, transpose,
song placement or notation editor in this checkpoint. Those have their own
sub-stages. UX selection remains before 0.5d, and is not a model blocker.

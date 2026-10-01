# 0.4c debugging

## 0.4c-fix1 — accompaniment and material selection

Base: PR #48 head `c65f79a`. Acceptance in Studio Pro is pending.

- The chord layer always contains all written tones, regardless of omissions in the selected solo source. Omitted notes are not added to the source layer.
- Non-chord slash bass is retained as a separately spelled `bassTone` in accompaniment and actual next-chord material. It does not change the chord's written degrees or become a guide automatically.
- A sus/power chord without a supported source still displays its literal accompaniment even though the compact source list is empty.
- Identical sources across interpretations share one explanation item. Every interpretation and its distinct application conditions remain attached; spelling, note roles, actual targets and tension classification stay part of material identity.
- A single melodic-minor list row exposes a button switching between the full seven-note source and the classified four-note T1 m6. The manual row label is a preference for the parent source and never classifies the full scale automatically. Text and viewer follow the chosen source form together.
- External anchors use source-relative pitch intervals on the staff, avoiding a pitch/register mismatch when the source root differs from the chord root.

`MaterialRegressionTests` covers these cases, including all twelve transpositions of the dominant example. Existing suites remain required.

Live check: G7→Cmaj7, choose Ab melodic minor and the chord layer (G B D F); switch D melodic minor between full scale and T1 Dm6 (C# absent from T1); check G7sus4, D5 and Cmaj7/F#; Fm7 in C major must show F melodic minor once with both interpretations. Seek/PLAY/STOP should keep text and viewer synchronized.

## 0.4c-fix2 — explanations and source references

Includes fix1. Acceptance in Studio Pro is pending.

- Selected material and the source catalog show every retained application condition. Identical catalog sources are grouped with all interpretation indices; conditions are not silently discarded.
- Text distinguishes an unplayed expected **major I** from a confirmed resolution to the real minor chord. The diagnostic explanation uses the same distinction.
- Melodic-minor, diminished and whole-tone references point to Levine chapter 3 in the supplied edition; printed page ranges are retained.
- [SOURCE_REFERENCE_CORRECTIONS.md](SOURCE_REFERENCE_CORRECTIONS.md) supersedes erroneous/unconditional rows of the supplied generated chart, including III → ♭III for melodic minor on half-diminished chords. The existing Core interval was already correct.

Live check additionally: Em7–A7–Dm7 must retain confirmed A7→Dm7 while calling the expected D-major I unplayed; select F melodic minor on G7 and inspect its conditional-overlay warning; compare merged F melodic minor on Fm7 with both retained interpretation conditions.

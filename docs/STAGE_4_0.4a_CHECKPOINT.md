# 0.4a — Visual Material Viewer (live-test pending)

## Scope

The viewer projects the selected `ExplanationItem` and its originating
`ImprovisationStrategy` from Core. It presents an illustrative ascending source
on a treble staff without rhythm, and every matching position on a standard
six-string guitar in the chosen fret range. The source is not a generated phrase
or a suggested fingering. The next chord's explicit target notes are separate.

Select a strategy, a visible layer (all/source/chord/guides/characteristic/next
targets), and frets 0–12, 5–17, or 12–24. A legend describes the dot colors;
target notes have purple outlines on the guitar. Note labels retain source
spelling (including enharmonic variants). The selected strategy's explanation,
provenance, context and existing missing/implied/contradicted markers appear
above the detailed text. No tension levels are classified in this checkpoint.

The ARA timer rebuilds the Core result at the host's current PPQ and refreshes
the viewer and text together. A selection is retained while its material still
exists, and falls back to the first item after a context change. When Core
leaves the primary interpretation unresolved, the first item is the shared
chord anchor; alternative interpretations are explicitly labeled and selectable.
Selection changes presentation only and never updates the harmonic ranking.

## Regression

- Host-neutral projection covers spelling `Db` versus `C#`, role filters,
  independent anchors, candidate provenance, next-chord targets and invalid
  selections (`MaterialViewerTests.cpp`).
- All existing Core/Context tests are run alongside the new test in Windows CI.
- Studio Pro ARA seek/transport and actual rendering require the Windows live
  test; no acceptance is recorded until Vlad confirms it.

## Live checklist — Fender Studio Pro

1. Install `Smart Improviser.vst3` from the `Smart-Improviser-0.4a-Windows`
   artifact and open the existing ARA test song. Confirm the header says `0.4a`.
2. On a simple `Dm7 → G7 → Cmaj7`, check the source labels on staff and fretboard,
   the guide tones, and the target notes of the **actual next chord**. Switch
   layers and each fret range; note positions must match the guitar.
3. Select a scale and then another strategy. The source name, notes, provenance
   and the explanation above the text must switch together. The treble staff
   has no durations or rhythm.
4. Seek across several chord boundaries, PLAY, then STOP and seek again. The
   viewer, current chord and explanation must stay in sync without a stale
   source or target.
5. Check a global/local/modal example and an ambiguous passage. Every
   alternative must remain selectable, without a UI-declared winner. Compare
   `Db`/`C#` spelling where available.
6. Check the earlier Corcovado rootless/implied and incomplete-cadence cases:
   gray missing, amber implied and red contradicted explanations remain intact;
   an expected tonic must not become an actual target note.

Status: **implementation checkpoint; awaiting Windows CI and Studio Pro live
acceptance**. Issue #5 and stable 0.4 remain unchanged until live confirmation.

## 0.4a fix1 — Enharmonic notation (2026-09-29)

Studio Pro screenshot on C7 → Fmaj7 showed `Db melodic minor` with `Eb`
on the source staff, while important chord-relative tones used `D#` and some
unspelled guides appeared as numeric pitch classes. `Eb` is correct as degree 2
of Db melodic minor, and `D#` is correct as #9 of C7. Neither source nor chord
spelling is changed by harmonic re-analysis.

The viewer now shows both spellings with an explicit equivalence label. The
source/all layers use source spelling; chord/guide/characteristic layers use the
already supplied chord-relative spelling. Staff position follows the displayed
letter, and the fretboard keeps the same pitch. Material and Sources text label
the two contexts. Important notes, anchor/guide/characteristic/target lists,
resolution moves, and evidence no longer fall back to a flat-biased pitch-class
list or raw numeric pitch classes. Existing Core spelling data and host degrees
take precedence; unknown degrees use the existing chord-relative display
convention without changing pitch or harmonic interpretation.

Live check: on `C7` choose `Db melodic minor`. Confirm `Eb` in the source layer,
`D#` in the characteristic layer, `Eb = D#` in the legend line, and identical
guitar positions. Check `Fb = E` on the same source. Inspect important notes,
guide/target lists and movement arrows in Material / Sources. Repeat on a sharp
key and a flat key to catch a fixed-name fallback. **Acceptance still pending.**

## 0.4a fix2 — V material on an incomplete ii–V

The accepted Stage 3 model already preserves `Dm7–G7 → D7/A` with missing C
and `Fm7–Bb7 → Em7` with missing Eb. Source gates had required a confirmed
major target, so the dominant lost its scale applications. The existing major-V
source catalog now applies **provisionally** to the actual V of this recognized
incomplete pattern, subject to explicit-chord compatibility. This is a material
option, not a change to the harmonic reading:

- The expected I is labeled missing and never becomes an actual next chord,
  confirmed resolution or established local key.
- The next-chord targets and optional movements continue to use the played
  `D7/A` or `Em7`, not the absent tonic.
- The UI names the missing I, keeps alternative interpretations, and labels
  the material's provisional provenance. No tension classification is added.
- Unknown future, actual major-I resolution and minor-target cases retain
  their existing rules. Explicit incompatible chord tones still veto a source.

Live check: seek to `G7` in `Dm7–G7–D7/A` and `Bb7` in `Fm7–Bb7–Em7`.
Select a provisional source, confirm the expected C/Eb is marked absent, and
verify the purple target notes still belong to real D7/A/Em7. Seek to the
next chord; the missing-I label must disappear. Compare a complete `ii–V–I`
and an `ii–V` whose future is unknown. **Live acceptance still pending.**

## 0.4a upd1 — Levine source rules (2026-09-30)

Mark Levine, *The Jazz Theory Book*, is the primary chord-scale reference for
this revision. The T1–T3 ladder is Vlad's product model, not a classification
from Levine; no tension algorithm or new harmonic interpretation is added.
Core now distinguishes minor-seven from minor-major-seven: the melodic-minor
source appears for an explicitly written `m6` or `m(maj7)`, while a bare `m7`
keeps its actual ♭7 and available diatonic material. A `G7b9` can expose
G half-whole diminished; an explicit `G7#5` without natural fifth can expose
G whole-tone. These are separate from whole-half on `dim7`, Lydian dominant
from D melodic minor and G altered from A♭ melodic minor. F melodic minor
on G7 is still a future, conditional overlay; it is not mislabeled as G altered.
Core keeps the actual target, explicit extensions, slash bass, ambiguous
interpretations and provisional provenance of a recognized incomplete ii–V.

Live checklist for upd1:

1. `Dm7 → G7b9 → Cmaj7`: select `G half-whole diminished`; check staff/frets
   and spelling `G Ab A# B C# D E F`. Select the original source to compare.
2. `Dm7 → G7#5 → Cmaj7`: select `G whole-tone`; check `G A B C# D# F`.
   A chord explicitly containing natural D must not show this source.
3. Compare a bare `Gm7` with explicitly written `Gm6` and `Gm(maj7)`:
   melodic minor appears only on the latter two. Compare the chord header.
4. Seek/play/stop across these chords. Source, staff, fretboard and written
   explanation must change together. The previous fix1 enharmonic test and
   fix2 incomplete `ii–V` test remain part of live acceptance.

Regression: all 16 host-neutral suites pass locally. Windows CI and Studio
Pro live acceptance remain pending; stable `0.4` and Issue #5 are unchanged.

## 0.4a upd2 — Dominant palette / Corcovado live correction

The `0.4a upd1` Windows Build #376 passed, but Vlad's Studio Pro screenshots
exposed a real gap: `D7/A → Abdim` correctly reads as `V/V → implied V`, yet
the source selector held only the chord anchors. The catalog had equated
availability of a scale with confirmation of a major/minor tonic. Levine's
half-whole dominant language had also been restricted to a written `7b9`,
so no ordinary dominant offered that optional color.

`upd2` separates chord-local options from function-specific interpretations:

- A written dominant compatible with Mixolydian gets a chord-local baseline
  when no interpretation-specific/basic provisional source exists. The label
  says "от аккорда", not "общая опора"; it establishes no tonic or local key.
- Half-whole diminished may be selected as an optional alteration on a
  compatible dominant even without explicit b9. Natural 9, b13 or an
  incompatible slash bass veto the complete collection. The original chord
  symbol is retained, and T1–T3 is not classified.
- An explicit augmented dominant without natural fifth may show whole-tone
  in a dominant chain. On `m7`, melodic minor returns as a marked **optional
  overlay**: actual b7 stays an anchor and source major 7 is passing; this
  is not the native m(maj7) chord-scale reading. Plain relative vi7 in major
  remains guarded.
- Selected-source text and Sources panel distinguish independent *scale*
  from independent *chord anchors*; a symmetric scale without an m6 thinking
  structure no longer prints `(no chord)` as if it were a thinking chord.

The [harmonic thinking map](HARMONIC_THINKING_MAP.md) inventories every
recognized pattern, current source coverage, explicit-chord guards and
subsequent work. It does not add new Harmonic Engine interpretations.

Live checklist for upd2:

1. On the screenshot's `D7/A → Abdim`, select `D Mixolydian • от аккорда`
   (`D E F# G A B C`) and `D half-whole diminished • от аккорда`
   (`D Eb F F# G# A B C`). Both contain bass A; purple targets belong to
   written `Abdim`. Neither selection establishes G major/minor as tonic.
2. Compare `G7 → Cmaj7` with `G7b9 → Cmaj7`: half-whole is an **optional**
   choice on both; G7 keeps its actual symbol, while explicit b9 excludes
   Mixolydian. `G9` blocks half-whole; `G13` may keep it because the scale
   contains natural E, but `G7b13` blocks it. Explicit natural 13 blocks
   altered, not half-whole.
3. Check `C7 → Fmaj7`, `A7b13 → Dm7`, and a dominant chain. No existing
   options or alternative interpretation may vanish. Verify `m7` melodic
   color is described as an overlay with a passing major seventh.
4. Seek, PLAY and STOP across `D7/A → Abdim → Gm7 → C7 → Fmaj7`; staff,
   fretboard, current chord, source label and text must remain synchronized.

`0.4a` still awaits Vlad's live acceptance; stable remains `0.4`.

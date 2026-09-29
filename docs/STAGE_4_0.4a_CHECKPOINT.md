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

# 0.4c — Level 1 / Stable (live test pending)

Stage 4 Issue #5 remains open; stable is `0.4`. This checkpoint applies the
accepted 0.4b profile contract to an already analyzed Core result. The
`analyzeStableTension` policy does not detect a key, reinterpret a chord or
replace a missing resolution. T2 and T3 remain `notEvaluated`.

## Rules

| Context from Core | T1 material | Roles and limits |
|---|---|---|
| Confirmed ordinary V7 → major, compatible written chord | Four tones of Core's m6 thinking structure (G7→Cmaj7: D F A B) | Written G is an external chord anchor. A is the natural 9; the source's E may be a passing approach, while C# from full D melodic minor is outside T1. Ordinary Mixolydian remains a separate baseline source. |
| Compatible major/minor chord with a confirmed diatonic source (Ionian, Dorian, Aeolian) | Literal chord notes plus natural 9, as a subset of that source | Major 4 against the third is only a passing note, not a stable source member. |
| Every other valid written chord, including incompatible alterations, sus, slash bass, unknown future and unsupported dominant resolution | Literal Core chord anchors and guides | Do not infer a replacement scale, tonic or natural fifth. |

All candidates retain the source's spelling, global/local/modal interpretation
index, evidence, actual chord, next chord and target. Multiple eligible sources
remain separate alternatives. A written alteration remains part of the
accompaniment even when a four-note thinking structure is used. The
`stableExtension` role is explicitly assigned, not inferred from scale
membership. `passingApproach` is an optional role and is outside the four or
five selected stable notes. This is Smart Improviser's own T1 policy, not a
T1 label in Levine's book.

The viewer offers T1 items alongside the original Core material. Selecting a
T1 item updates staff, fretboard, selected material and explanation together
with seek/PLAY/STOP. The m6 source layer contains four notes; chord/all layers
also expose the literal dominant root. Text labels T1. This checkpoint does
not add rhythm, phrase entry, persistence, or T2/T3 algorithms.

## Verification and live gate

`TensionLevel1Tests` covers G7→Cmaj7, V→minor, written b9 and #5,
unknown next chord, Dm7/Cmaj7 natural 9, passing major 4, sus, preserved
ambiguous context and the viewer's chord/source split. All previous test
suites and Windows CTest must pass. The Windows workflow packages
`Smart-Improviser-0.4c-Windows` VST3.

Studio Pro live check: install the VST3; confirm version `0.4c`. At
G7→Cmaj7, select `T1 • Dm6` and compare four source notes D F A B and G in
the chord layer. Check G7b9 and G7#5 do not offer ordinary Dm6, and an
unknown next chord shows literal anchors. Seek across chords and PLAY/STOP:
selected text, staff and fretboard should follow together. Acceptance remains
pending Vlad's live confirmation.

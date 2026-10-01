# 0.4c — Level 1 / Stable (live test pending)

`0.4c-fix3` includes the accompaniment, source grouping, T1 source-form switch and explanation, contextual-foundation and minor-destination corrections in [STAGE_4_0.4c_FIXES.md](STAGE_4_0.4c_FIXES.md). The source-form button makes the four-note subset accessible without another compact list row; full scales are not automatically T1.

Stage 4 Issue #5 remains open; stable is `0.4`. This checkpoint applies the
accepted 0.4b profile contract to an already analyzed Core result. The
`analyzeStableTension` policy does not detect a key, reinterpret a chord or
replace a missing resolution. T2 and T3 remain `notEvaluated`.

## Rules

| Context from Core | T1 material | Roles and limits |
|---|---|---|
| Confirmed ordinary V7 → major **or** ordinary `iim7–V7` with provisional major direction, compatible written chord | Four tones of Core's m6 thinking structure (G7→Cmaj7: D F A B; Fm7–B♭7–Em7: F A♭ C D on B♭7) | The written dominant root is an external chord anchor. The full melodic-minor scale is not T1. For incomplete ii–V, an expected I remains unplayed and only the actual next chord supplies factual targets. Ordinary Mixolydian remains a separate baseline source. |
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

The viewer offers T1 items alongside the original Core material. The current
context shows the harmonic context on the left and every available material
in a compact, vertically scrollable column on the right. The former
"Thinking" line is removed; the source list carries that choice without
repeating the expected/missing tonic status in each row. Detailed evidence
remains in the explanation. The column orders rows by manually assigned
T1, T2, T3, then unmarked; colored circles and fine separators distinguish
groups without taking a row for a heading. The panel height is reduced.
Core's compatible diatonic mode appears above these groups as the fixed
foundation, without a tension selector. The literal chord-tone source remains
available on the fretboard and in Core but is hidden from this compact column.
The chord-anchor T1 candidate also remains in Core but shares the written
chord's displayed notes; its redundant list row is suppressed.
Selecting a row updates staff, fretboard and explanation together with
seek/PLAY/STOP. A separate circle beside each row opens a colored menu for a
**manual** T1/T2/T3 label or no label. The manual label never reclassifies Core
evidence, changes the source, or declares a tonic; unmarked is the default.
These UI labels and window dimensions are saved in the plugin instance's host
state and restored when the host restores the plugin. This is not a saved Song
workspace. Core's T2/T3 algorithms remain for later checkpoints.
The fretboard button switches between note names and Arabic degrees relative
to the actual written chord, including explicit alterations such as #5 and
b13. It does not change the staff or harmonic analysis. The display choice is
stored in V3 host state; V2 projects still restore their tension labels and
window size, with note names as the default.

Manual labels now use a transposition-independent key: the Core source rule,
source-root interval over the written chord, chord quality/tones/slash bass and
harmonic role. Thus equivalent major ii–V contexts, including completed and
incomplete turns and local key changes, share a label without assigning the
same label to unrelated dominant or minor situations. This affects new labels;
old per-chord keys in existing host state are harmless but cannot be inferred
as general preferences. The four-note T1 m6 remains a separate Core subset of
the full seven-note melodic minor. When both have the same root, the compact
UI displays the full source once; its details still describe source notes and
Core keeps the four-tone safety contract.

Five tabs now sit above the upper panel: Current Context, Material, Sources /
Notes, Harmonic Analysis and ARA / Diagnostics. The context tab shows the
current chord in yellow and a larger Roman pattern with the corresponding
written chord names beneath its members; an unplayed expected tonic gets a
grey dash. The other tabs show their scrollable text in the same upper panel.
The staff and fretboard stay visible underneath. The viewer is trimmed to its
actual content height, leaving a larger, empty bottom panel reserved for a
future notation editor. No note or rhythm editing is added in this checkpoint.

In an incomplete ordinary major ii–V, Core no longer offers a parallel-minor
modal candidate solely from the ii chord. It provides provisional Dorian on
ii and Mixolydian on V; the global key and any unconfirmed SubV hypothesis are
kept for diagnostics, not the compact playing function. The actual next chord
remains the only factual target.
This palette also survives a real V-to-minor resolution when the preceding ii
is the ordinary m7 form: Em7–A7–Dm7 receives the same source family as the
transposed Dm7–G7–D7/A, while Dm7 remains the actual minor target and the
expected major I is unplayed. The rule is interval based in every key, and
explicitly altered chords still filter incompatible sources. A real minor
resolution with written b9/b13 can retain its compatible minor V fragment.

The editor window can be resized within fixed bounds; fonts, controls and
noteheads retain their pixel size as available space changes. The fretboard
draws a clear nut between open strings and fret 1, with Roman position markers
only over frets 3, 5, 7, 9, 12, 15, 17, 19 and 21 when visible. The m6 source
layer contains four notes; chord/all layers also expose the literal dominant
root. This checkpoint does not add rhythm, phrase entry or song persistence.

## Verification and live gate

`TensionLevel1Tests` covers G7→Cmaj7, V→minor, written b9 and #5,
Fm7–B♭7–Em7 with missing I, candidate ii–V with unknown future, isolated
dominant with unknown future, Dm7/Cmaj7 natural 9, passing major 4, sus,
preserved ambiguous context and the viewer's chord/source split.
The incomplete-turn regression also checks that C minor is not inferred from
Fm7 in this explicit ii–V and that F Dorian remains available. Relative manual
keys are compared across all twelve transpositions and completed/incomplete
turns.
`FretboardLayoutTests` checks nut/0/1 geometry and Roman markers. All previous test
suites and Windows CTest must pass. The Windows workflow packages
`Smart-Improviser-0.4c-Windows` VST3.

Studio Pro live check: install the VST3; confirm version `0.4c`. At
G7→Cmaj7, select D melodic minor and compare its full seven-note source with
the written G chord layer; Core's four-note Dm6 subset remains covered by
`TensionLevel1Tests`, without a duplicate list row. For Fm7–B♭7–Em7, confirm
F Dorian on ii, a provisional major-V palette on B♭7 without E♭ as a played
tonic or target, and no C-minor reading. Check G7b9 and G7#5 do not offer
ordinary Dm6. Assign and clear
colored labels to two rows, seek away/back and save/reopen the Studio Pro
project. Confirm that equivalent ii–V turns in another key inherit the labels,
that Fm6 is not repeated next to F melodic minor, and that the compact panel
has no group headings. Resize the editor; compare fixed text/notehead size. In ranges 0–12,
5–17 and 12–24, check the nut and Roman positions. Seek and PLAY/STOP should
keep selected text, staff and fretboard synchronized. Acceptance remains
pending Vlad's live confirmation.

Latest UI check: the arrows around the highlighted current chord render
correctly; a compatible mode such as G Mixolydian is first and cannot receive
a T1–T3 label; G7 chord tones are absent from the strategy column. Toggle
fretboard labels and compare G7#5 (#5) with G7b13 (b13), then save and reopen
the host project to check the choice is restored.

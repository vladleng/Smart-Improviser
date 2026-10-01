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

## 0.4c-fix3 — contextual foundation and minor destination

Includes fix2. Studio Pro acceptance is pending.

- A compatible diminished triad now offers its root's whole–half collection, as does dim7. The written chord remains a triad; a scale tone does not become a written seventh. Custom key context is supported without inventing a tonal interpretation.
- The added natural ninth in the T1 chord + 9 subset is marked characteristic in both source and chord-relative material, and appears in the characteristic viewer layer. Chord anchors and guide tones remain separate.
- The compact list selects one diatonic foundation: a confirmed local turn takes precedence, then an incomplete-turn hypothesis, then global context. Other interpretations remain in diagnostics. Interpretation-number suffixes are removed from normal source labels.
- In **Em7–A7(b13)–D7, global C major**, the ii–V root direction and contextual minor triad support **hypothetical Dm**. D7 is retained as the actual next chord; no Dm resolution or established D-minor key is asserted. A played major tonic overrides the hypothesis. The original major ii–V template remains structural diagnostic evidence; it does not determine the contextual destination's quality.
- **D harmonic minor** is added as the primary conditional source: D E F G A Bb C#, or 1 b9 3 11 5 b13 b7 relative to A. The 11 needs passing treatment against C#; holding b13 depends on the melody. The full seven-note source is separate from the existing six-note harmonic-minor V fragment and is not automatically T1.
- **Bb melodic minor** remains the altered alternative. **D melodic minor** remains a rare supplementary fifth-mode color, with natural 9 on A; it does not replace harmonic minor. Unclassified/manual source ordering retains the primary-to-supplementary priorities.
- These rules use transposed intervals and explicit degrees. An isolated b13 cannot create a minor target; an incompatible tonal context or written natural 9 blocks the full harmonic-minor source. An explicit #5 is not silently relabelled b13 for this rule.

Validation: `Fix3RegressionTests` covers the four cases across all twelve transpositions, local F-major foundation and characteristic ninth, factual versus hypothetical targets, custom key context and negative cases. Run all 21 host-neutral suites plus the Windows plugin build.

Live check: Abdim between D7/A and Gm7 must offer Ab whole–half; Fmaj7 + 9 after Gm7–C7 must show G in “Характерные”; Dm7 before G7 must have one Dorian foundation; A7(b13) in Em7–A7(b13)–D7 must show presumed Dm, full D harmonic minor before altered/rare colors, while the next-chord layer continues to show D7. Repeat seek/PLAY/STOP and reopen.

## 0.4c-fix4 — shared minor-dominant destination and diminished arpeggio

Includes fix3. Studio Pro acceptance is pending. This section supersedes fix3's separation of the harmonic-minor fragment and its preceding-ii eligibility gate.

`DominantDestination` separates the destination used for source selection from the factual next chord and Stage 2 resolution/key evidence. A confirmed major/minor resolution takes precedence; otherwise written major/minor continuation supplies its quality without asserting tonicization. A changed-quality continuation on the expected root can support a presumed minor destination from established local or global tonal context and explicit b9/b13. The preceding ii quality is not an eligibility gate. A mismatched continuation root or unsupported tonal context does not create that hypothesis.

- Em7–A7(b13)–D7 and Em7b5–A7(b13)–Dm7 receive the same four sources: D harmonic minor, Bbdim7 arpeggio, Bb melodic minor (altered) and the rare supplementary D melodic minor. Confirmed versus presumed destination changes evidence/conditions, not the source collection.
- The separate six-note `harmonic-minor V fragment` rule and its label are removed. Harmonic minor is always named from the destination and offered as the full seven-note collection where compatible, including ordinary V7 resolving to minor.
- The diminished source here is an **arpeggio**, not an octatonic scale. bII dim7 gives b9–3–5–b7: over A, Bb C# E G (native Bb arpeggio spelling: Bb Db Fb Abb). It introduces no F# / natural 13. Literal A and written F / b13 stay in the chord layer, outside the four-note source. Its thinking structure is Bbdim7.
- Actual D7 remains in the timeline and next-chord layer. The displayed Em7–A7 direction has an absent minor i; D7 is not inserted as its tonic or used to establish D major. The same display also works with a half-diminished ii and presumed minor destination.
- The minor destination overrides the old major ii–V playing palette even when the preceding ii is ordinary m7. The old major template is retained only as explicitly labelled structural diagnostic evidence; its absent I does not override an actual minor resolution.
- Manual tension keys use the shared minor destination before the template/actual-continuation category, so matching sources in both cases share a label. Full sources and the new arpeggio are not automatically classified T1.
- Compatibility remains source-specific: written natural 9, incompatible #5/b5 spelling or slash bass cannot be silently omitted from an incompatible collection. Natural-9 chords can retain other compatible sources. The unchanged full half–whole diminished scale still excludes written b13.

`Fix4RegressionTests` compares complete source signatures, common manual labels, four-note arpeggio/viewer projection, destination evidence and negative cases across all twelve transpositions, with minor ii, half-diminished ii and no preceding ii. Existing suites cover major, SubV, incomplete and unknown-future contexts. All 22 suites and the Windows plugin build are required.

Live check: compare the two supplied A7(b13) positions; D harmonic minor must replace the fragment in both, Bb diminished arpeggio must be selectable, and its source layer must contain four tones while the chord layer keeps A and F. D7 stays out of the first displayed turn; Dm7 remains the confirmed arrival in the second. Labels, source selection and notes must remain consistent during seek/PLAY/STOP/reopen.

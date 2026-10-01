# Corrections to the supplied reference chart

The generated image dated 2026-09-29 is a preliminary mnemonic, not an
unconditional compatibility table. The following corrections supersede its
corresponding rows; musical rules must use written notes, bass and actual
continuation. Pentatonic and sus applications in the image are not an
implemented source catalog in 0.4c.

| Entry | Correct reading | Evidence / limitation |
|---|---|---|
| Melodic minor on a half-diminished chord | Start at **♭III**, not III. Bm7♭5 → D melodic minor; Em7♭5 → G melodic minor. | Boyko, section 2, printed p. 93, examples 143–146 explicitly say lowered third; Levine, printed pp. 101–103. Core's `boyko.melodic-minor.bIII` already uses this interval. |
| Melodic minor on G7alt | A♭ melodic minor is the altered collection. F melodic minor is a separate conditional project overlay. | The F source includes natural 5/11/13 and omits B, the written dominant third; do not call it G altered. |
| Diminished whole–half from ♭II over a dominant | A♭ whole–half and G half–whole have the same pitch classes. | G A♭ B♭ B D♭ D E F includes natural 5 and 13; it is not compatible with every written G7alt. Explicit natural 9 or ♭13 excludes the full collection. |
| Whole-tone from II over a dominant | A whole-tone and G whole-tone have the same pitch classes. | G A B C♯ D♯ F contains natural 9, omits natural 5, and represents #5; it is not a universal choice for written ♭9/#9 or an arbitrary 7alt voicing. |
| Minor pentatonic from I on a dominant | Optional blues/minor-third overlay, not a complete literal dominant source. | G minor pentatonic contains B♭ and no B. Preserve the written major third separately and state the intended blues context. |
| Minor pentatonic from VII on major | Conditional Lydian color, not an unconditional Ionian source. | B minor pentatonic over Cmaj7 contains F♯ (#11). Melody/voicing and intended color matter. |

## Levine chapter numbers

In the supplied five-part edition, printed pp. 88–127 belong to **Chapter 3,
Chord/Scale Theory**, including melodic-minor, diminished and whole-tone
harmony. Chapter 4 is How To Practice Scales; Chapter 5 is Slash Chords.
Chapter 23, pp. 529–531, remains the reference for harmonic-minor fragments.
Page numbers above refer to the printed edition, not to an arbitrary PDF split.

This document corrects project references; it does not attribute Smart
Improviser's T1/T2/T3 policy to either author or alter the uploaded books.

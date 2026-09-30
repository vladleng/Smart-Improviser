# 0.4b — Tension Engine contract (live test pending)

`0.4a upd4` was accepted by Vlad and merged through PR #45. Stable remains
`0.4`, Stage 4 Issue #5 stays open. This checkpoint adds the Core contract
for applying tension policies; it does not implement T1/T2/T3 musical rules.

## Data boundary

`TensionLevel` keeps its existing numeric values 1/2/3. `TensionRole` describes
the job of an individual pitch within an applied strategy: chord anchor,
guide tone, contextual color, passing/approach, resolution target or outside
tone. `TensionRoleAssignment` contains a `MaterialNote` and explicitly scopes
it to the current or next chord. These are instrument-neutral terms; role
does not follow automatically from pitch class or scale membership.

`buildTensionProfile(ImprovisationResult)` produces an ephemeral profile of
the **already analyzed** `HarmonicSituation`. It copies the real current/next
chords, resolution, key centers, interpretations and confidence without
reanalyzing them. Each level has an independent state:

| State | Meaning |
|---|---|
| `notEvaluated` | No policy has classified the level; a default enum value is not T1 evidence. |
| `supported` | A policy has attached one or more classified strategy alternatives. |
| `unavailable` | A later policy explicitly explains why the level cannot be offered. |

Only strategies marked `tensionClassified` enter a supported band. The profile
keeps **all** classified alternatives and their original `ruleId`, harmonic
evidence, source spelling, real target and interpretation index. An unresolved
primary remains unresolved. Empty bands stay `notEvaluated` in 0.4b; they are
not inferred to be `unavailable`, nor filled from a source name or the
priority of a suggestion. Classification and role assignments begin in
`0.4c–0.4e`; target-aware policy in `0.4f`, selection/UI in later checkpoints.

This situation profile is distinct from a future descriptive tension profile
inside a saved Phrase and from a song's TensionCurve. It is rebuilt with each
analysis result and cannot mutate a phrase or the ARA harmony.

## Regression and live gate

The dedicated `TensionContractTests` checks numeric levels, unknown levels,
default roles, a confirmed `G7→Cmaj7`, unknown future, invalid context and
two classified alternatives with no primary interpretation. The 16 previous
host-neutral suites must remain green; Windows CI builds and packages the
VST3 separately as `Smart-Improviser-0.4b-Windows`.

Studio Pro smoke check for 0.4b: replace the VST3, verify the version label
`0.4b`, compare `G7→Cmaj7`, `G7→Cm7` and an unresolved/unknown continuation
while seeking and playing. Existing source notes, real targets, legend and
text must remain synchronized; no T1/T2/T3 classification or control is
promised in this contract checkpoint. Vlad's live acceptance is still needed.

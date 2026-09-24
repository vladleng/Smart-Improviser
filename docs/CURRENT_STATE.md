# Smart Improviser — Current State

> Короткая точка входа для нового чата или рабочей сессии. Подробная архитектура — в `PROJECT_CONTEXT.md`, этапы — в `ROADMAP.md`, правила версий — в `VERSIONING.md`, фактический прогресс — в GitHub Issues.

## Текущее состояние

- **Завершённый Stage:** Stage 1 — ARA Context Monitor
- **Текущая стабильная версия:** `0.2`
- **Активный Stage:** Stage 2 — Harmonic Engine
- **Последний принятый checkpoint:** `0.2f — Integration / musical validation`
- **Следующий шаг:** стабильная `0.3 — Stage 2 complete`
- **Stage 2 Issue:** #3 — Stage 2 — Harmonic Engine
- **PR:** #20 — `0.2f — Integration / musical validation`

`0.2f` принят после полного Windows CI и live musical validation в Fender Studio Pro. Это последний буквенный checkpoint Stage 2 перед стабильной `0.3`.

## Архитектурная граница

Stage 1 остаётся закрытым. Harmonic Engine по-прежнему получает только host-neutral:

```text
previous chord
current chord
next chord
global key
```

Project key в DAW автоматически не меняется. Core остаётся независимым от JUCE / ARA / Fender Studio Pro.

Цепочка Stage 2:

```text
TimelineHarmonicSnapshot
        ↓
Global harmonic analysis
        ↓
Pattern Recognizer
        ↓
Tritone Substitution
        ↓
Local Key Center Analyzer
        ↓
Ambiguity / Confidence Analyzer
        ↓
HarmonicSituation
```

## Принятые возможности Stage 2

К завершению `0.2f` приняты и live-tested:

- basic harmonic functions;
- major `ii–V–I` на `ii / V / I`;
- minor `iiø–V–i` на `iiø / V / i`;
- `V–I`;
- `I–VI–ii–V`;
- secondary dominants и dominant chains;
- ordinary `V7` / `SubV7`;
- major/minor `ii–SubV–I`;
- applied SubV и guide-tone resolution;
- candidate / tonicized / established / modulationCandidate local centers;
- global/local harmonic interpretations;
- borrowed/modal ambiguity;
- `unique / ambiguous` state и explicit primary interpretation;
- enharmonic-aware KeyCenter display через `rootFifths`;
- integration false-positive guard для известного противоречащего `next` chord.

## 0.2f — Integration / musical validation [ACCEPTED]

Добавлен отдельный integration regression target:

```text
SmartImproviserIntegrationValidationTests
```

Он перемещает окно `previous / current / next` по длинным progression cases и проверяет transitions между global/local/ambiguous состояниями.

### Принятые progression cases

1. **Global turnaround + temporary tonicization + return**

```text
C major
Cmaj7 → A7 → Dm7 → G7 → Cmaj7
```

Подтверждено:
- `I–VI–ii–V`;
- `A7 → Dm` как temporary D minor tonicization;
- на `Dm` local center ещё сохраняется как подтверждённая тонизация;
- на `G7 → Cmaj7` происходит возврат к `confirmed / unique / Global` без local center.

2. **Borrowed/modal ambiguity → global resolution**

```text
C major
Cmaj7 → Fm7 → G7 → Cmaj7
```

Подтверждено:
- `Fm7` = `AMBIGUOUS`, Global C major + Modal interchange C minor;
- `G7 → Cmaj7` = `confirmed / unique / Global`.

3. **Local iiø–SubV–i inside global context**

```text
C major
Em7b5 → Eb7 → Dm → G7 → Cmaj7
```

Подтверждено:
- local D minor candidate / tonicized context;
- `iiø–SubV–i`;
- Local primary на Dm;
- последующий возврат в global C major на `G7 → Cmaj7`.

4. **Remote tonicization → modulationCandidate**

```text
C major
C#7 → F#maj7 → Bmaj7
```

Подтверждено:
- `C#7 → F#maj7` = confirmed F# tonicization;
- на `F#maj7` local center = `F# major | local | modulation candidate`;
- interpretation = `AMBIGUOUS / UNRESOLVED`;
- explicit global key остаётся C major.

5. **Boundary false-positive guard**

```text
C major
Dm7 → G7 → Abmaj7
```

Подтверждено: на G7 `Global pattern = None`, `Resolution = -`; ложный `I–VI–ii–V` не создаётся при известном противоречащем `next` chord.

6. **Stage 1 contract safety**

Missing position / missing key остаются штатными `NO ANALYSIS`, без invented pattern/local center.

## Версия принятого checkpoint

```text
Build label: Smart Improviser 0.2f
CMake:      0.2.8
Artifact:   Smart-Improviser-0.2f-Windows
Package:    Smart Improviser.vst3
Windows Build #217: SUCCESS
Tests:      7 / 7 PASS
```

## Линия Stage 2

```text
0.2a — Harmonic Engine foundation          [ACCEPTED]
0.2a fix1 — Harmonic Engine diagnostics    [ACCEPTED]
0.2b — Pattern Recognizer                  [ACCEPTED]
0.2c — Tritone Substitution                [ACCEPTED]
0.2d — Local Key Center                    [ACCEPTED]
0.2e — Ambiguity / Confidence              [ACCEPTED]
0.2e fix1 — Enharmonic spelling            [ACCEPTED]
0.2f — Integration / musical validation    [ACCEPTED]
0.3  — Stage 2 complete                    [NEXT]
```

## Acceptance 0.2f

- [x] все существующие Stage 2 regression tests зелёные;
- [x] `SmartImproviserIntegrationValidationTests` зелёный;
- [x] complex progression transitions соответствуют ожидаемой semantics;
- [x] boundary false-positive guards зелёные;
- [x] Windows CI зелёный;
- [x] artifact `Smart-Improviser-0.2f-Windows` опубликован;
- [x] live musical validation в Fender Studio Pro;
- [x] Stage 1 regression отсутствует;
- [x] checkpoint принят;
- [ ] подготовлена стабильная `0.3`.

## Следующий шаг — стабильная 0.3

`0.3` не должна добавлять новый музыкальный слой. Это релизное закрытие Stage 2: финальная версия, документация, artifact и проверка того, что все принятые checkpoints `0.2a…0.2f` представлены в `main` без новых изменений поведения.

## Что читать в новом чате Stage 2

1. `docs/CURRENT_STATE.md`;
2. `docs/STAGE_1_TO_STAGE_2_CONTRACT.md`;
3. `docs/CORE_DATA_MODEL_0.0b.md`;
4. `docs/PROJECT_CONTEXT.md`;
5. `docs/ARCHITECTURAL_DECISIONS.md`;
6. `docs/ROADMAP.md`;
7. `docs/VERSIONING.md`;
8. Issue #3 — Stage 2 — Harmonic Engine;
9. PR #20 — принятый `0.2f — Integration / musical validation`.

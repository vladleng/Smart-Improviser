# Smart Improviser — Current State

> Короткая точка входа для нового чата или рабочей сессии. Подробная архитектура — в `PROJECT_CONTEXT.md`, этапы — в `ROADMAP.md`, правила версий — в `VERSIONING.md`, фактический прогресс — в GitHub Issues.

## Текущее состояние

- **Завершённый Stage:** Stage 1 — ARA Context Monitor
- **Текущая стабильная версия:** `0.2`
- **Активный Stage:** Stage 2 — Harmonic Engine
- **Последний принятый checkpoint:** `0.2e / 0.2e fix1 — Ambiguity / Confidence`
- **Текущая рабочая версия:** `0.2f — Integration / musical validation`
- **Stage 2 Issue:** #3 — Stage 2 — Harmonic Engine
- **Активная ветка:** `stage-2-integration-validation`
- **Активный PR:** #20 — `0.2f — Integration / musical validation`

`0.2e / 0.2e fix1` принят после CI и live-test и слит в `main` через PR #19. `0.2f` является последним буквенным checkpoint Stage 2 перед стабильной `0.3`.

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

К началу `0.2f` приняты и live-tested:

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
- enharmonic-aware KeyCenter display через `rootFifths`.

## 0.2f — Integration / musical validation [ACTIVE]

Цель: проверить весь Stage 2 как единый Harmonic Engine на последовательностях, а не только отдельные анализаторы.

### Новый integration regression target

```text
SmartImproviserIntegrationValidationTests
```

Он перемещает окно `previous / current / next` по длинным progression cases и проверяет transitions между global/local/ambiguous состояниями.

### Интеграционные progression cases

1. **Global turnaround + temporary tonicization + return**

```text
C major
Cmaj7 → A7 → Dm7 → G7 → Cmaj7
```

Проверяется:
- global `I–VI–ii–V`;
- `A7 → Dm` как temporary D minor tonicization;
- Local center становится primary при confirmed tonicization;
- на `Dm7 → G7 → Cmaj7` происходит возврат к unique global context.

2. **Borrowed/modal ambiguity → global resolution**

```text
C major
Cmaj7 → Fm7 → G7 → Cmaj7
```

Проверяется:
- Fm7 = `AMBIGUOUS` global chromatic / C minor modal interchange;
- G7→C возвращает `UNIQUE / Global`.

3. **Local iiø–SubV–i inside global context**

```text
C major
Em7b5 → Eb7 → Dm → G7 → Cmaj7
```

Проверяется:
- local D minor;
- `iiø–SubV–i`;
- established local center / Local primary;
- последующий возврат в global C major.

4. **Remote tonicization → modulationCandidate**

```text
C major
C#7 → F#maj7 → Bmaj7
```

Проверяется:
- confirmed F# tonicization на dominant position;
- F# major становится `modulationCandidate` на resolution position;
- interpretation остаётся `AMBIGUOUS / UNRESOLVED`;
- explicit global key остаётся C major.

5. **Boundary false-positive guard**

```text
C major
Dm7 → G7 → Abmaj7
```

Если `next` уже известен и противоречит ожидаемому разрешению, движок не должен придумывать завершение `I–VI–ii–V` только по паре `Dm7 → G7`.

6. **Stage 1 contract safety**

Missing position / missing key остаются штатными `NO ANALYSIS`, без invented pattern/local center.

## Версия 0.2f

```text
Build label: Smart Improviser 0.2f
CMake:      0.2.8
Artifact:   Smart-Improviser-0.2f-Windows
Package:    Smart Improviser.vst3
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
0.2f — Integration / musical validation    [ACTIVE]
0.3  — Stage 2 complete
```

## Acceptance 0.2f

- [ ] все существующие Stage 2 regression tests зелёные;
- [ ] `SmartImproviserIntegrationValidationTests` зелёный;
- [ ] complex progression transitions соответствуют ожидаемой semantics;
- [ ] boundary false-positive guards зелёные;
- [ ] Windows CI зелёный;
- [ ] artifact `Smart-Improviser-0.2f-Windows` опубликован;
- [ ] live musical validation в Fender Studio Pro;
- [ ] Stage 1 regression отсутствует;
- [ ] checkpoint принят;
- [ ] подготовлена стабильная `0.3`.

## Что читать в новом чате Stage 2

1. `docs/CURRENT_STATE.md`;
2. `docs/STAGE_1_TO_STAGE_2_CONTRACT.md`;
3. `docs/CORE_DATA_MODEL_0.0b.md`;
4. `docs/PROJECT_CONTEXT.md`;
5. `docs/ARCHITECTURAL_DECISIONS.md`;
6. `docs/ROADMAP.md`;
7. `docs/VERSIONING.md`;
8. Issue #3 — Stage 2 — Harmonic Engine;
9. PR #20 — `0.2f — Integration / musical validation`.

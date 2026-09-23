# Smart Improviser — Current State

> Короткая точка входа для нового чата или рабочей сессии. Подробная архитектура — в `PROJECT_CONTEXT.md`, этапы — в `ROADMAP.md`, правила версий — в `VERSIONING.md`, фактический прогресс — в GitHub Issues.

## Текущее состояние

- **Завершённый Stage:** Stage 1 — ARA Context Monitor
- **Текущая стабильная версия:** `0.2`
- **Активный Stage:** Stage 2 — Harmonic Engine
- **Последний принятый checkpoint:** `0.2a / 0.2a fix1`
- **Следующая рабочая версия:** `0.2b — Pattern Recognizer`
- **Stage 2 Issue:** #3 — Stage 2 — Harmonic Engine
- **Рабочая ветка:** `stage-2-harmonic-engine`
- **PR checkpoint 0.2a:** #15 — `0.2a fix1 — Stage 2 Harmonic Engine diagnostics`

## Принятие checkpoint 0.2a

`0.2a` и `0.2a fix1` **приняты по результатам CI и live-test в Fender Studio Pro**.

Подтверждено:

- Windows Build #148 — success;
- artifact `Smart-Improviser-0.2a-fix1-Windows` установлен;
- Stage 1 ARA/context regression не обнаружен;
- Harmonic Engine diagnostics работают в реальном plugin UI;
- `Cmaj7` в C major → `I | Tonic`, Diatonic;
- `Am7` → `VI | Tonic`, Diatonic;
- `Dm7` → `II | Predominant`, Diatonic;
- `Dm7 → G7 → Cmaj7` на G7 → `V | Dominant`, `Major ii-V-I`, `Dominant | 2 / 3`, resolution `Cmaj7 | CONFIRMED`;
- `D7 → G7` в C major на D7 → Chromatic, `Secondary dominant`, local center `G major | temporary`, resolution `G7 | CONFIRMED`;
- confidence/evidence обновляются ожидаемо (`high/confirmed | unique`).

## Архитектурная цепочка

```text
Fender Studio Pro / ARA 2
        ↓
TimelineHarmonicSnapshot
        ↓
buildHarmonicSituation()
        ↓
analyzeHarmonicSituation() / Harmonic Engine
        ↓
HarmonicSituation
        ↓
Diagnostic UI / future product UI
```

Core и Harmonic Engine остаются host-neutral. UI только отображает уже рассчитанный `HarmonicSituation`.

## Что закрыто в 0.2a

- отдельный `HarmonicEngine`;
- global key + previous/current/next analysis;
- basic harmonic function;
- major `ii–V–I` для current V;
- minor `iiø–V–i` для current V;
- fallback `V–I`;
- secondary dominant;
- evidence-backed temporary local center;
- safe non-analysis states;
- regression tests;
- Stage 2 diagnostic UI.

## Следующий подэтап — 0.2b Pattern Recognizer

Цель: расширить pattern recognition с текущего минимального варианта до полноценного анализа позиции внутри оборота.

План:

- распознавать major `ii–V–I` для позиций `ii / V / I`;
- распознавать minor `iiø–V–i` для позиций `iiø / V / i`;
- добавить `I–VI–ii–V`;
- добавить secondary-dominant chains;
- добавить pattern positions: начало / середина / resolution;
- добавить pattern evidence/confidence;
- boundary regression tests.

## Дальнейшая линия Stage 2

```text
0.2a — Harmonic Engine foundation          [ACCEPTED]
0.2a fix1 — Harmonic Engine diagnostics    [ACCEPTED]
0.2b — Pattern Recognizer                  [NEXT]
0.2c — Tritone Substitution
0.2d — Local Key Center
0.2e — Ambiguity / Confidence
0.2f — Integration / musical validation
0.3  — Stage 2 complete
```

## Что делать следующим

1. начать реализацию `0.2b — Pattern Recognizer`;
2. не расширять `0.2a fix1` новой функциональностью;
3. сохранить host-neutral границу Harmonic Engine;
4. после завершения `0.2b` провести отдельный regression/live-test checkpoint перед переходом к `0.2c`.

## Что читать в новом чате Stage 2

1. `docs/CURRENT_STATE.md`;
2. `docs/STAGE_1_TO_STAGE_2_CONTRACT.md`;
3. `docs/CORE_DATA_MODEL_0.0b.md`;
4. `docs/PROJECT_CONTEXT.md`;
5. `docs/ARCHITECTURAL_DECISIONS.md`;
6. `docs/ROADMAP.md`;
7. `docs/VERSIONING.md`;
8. Issue #3 — Stage 2 — Harmonic Engine.

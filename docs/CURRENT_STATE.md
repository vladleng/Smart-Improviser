# Smart Improviser — Current State

> Короткая точка входа для нового чата или рабочей сессии. Подробная архитектура — в `PROJECT_CONTEXT.md`, этапы — в `ROADMAP.md`, правила версий — в `VERSIONING.md`, фактический прогресс — в GitHub Issues.

## Текущее состояние

- **Завершённый Stage:** Stage 1 — ARA Context Monitor
- **Текущая стабильная версия:** `0.2`
- **Активный Stage:** Stage 2 — Harmonic Engine
- **Последний принятый checkpoint:** `0.2d — Local Key Center`
- **Следующий checkpoint:** `0.2e — Ambiguity / Confidence`
- **Stage 2 Issue:** #3 — Stage 2 — Harmonic Engine
- **Принятый PR:** #18 — `0.2d — Local Key Center`

`0.2d` принят после успешного CI и полного live-test в Fender Studio Pro. Все заявленные checkpoint-проверки пройдены без обнаруженных проблем.

## Архитектурная граница

Stage 1 остаётся закрытым. `0.2d` не меняет ARA/context contract и не меняет project key в DAW:

```text
Fender Studio Pro / ARA 2
        ↓
TimelineHarmonicSnapshot
(previous / current / next)
        ↓
Harmonic Engine
        ├── global interpretation
        └── Local Key Center Analyzer
                ↓
          local interpretation
        ↓
HarmonicSituation
```

Core остаётся host-neutral.

## Принятый checkpoint 0.2d — Local Key Center

Главная идея: global key проекта остаётся стабильным, а движок отдельно определяет активный локальный/временный тональный центр.

Пример:

```text
Global key: F major
Em7b5 → A7 → Dm

Current: A7
Global interpretation: chromatic / applied dominant
Local center: D minor
Local function: V / Dominant
Local pattern: minor iiø–V–i
```

### Модель local center

```text
candidate
    ↓
tonicized / temporary
    ↓
established local center
    ↓
modulationCandidate
```

`modulationCandidate` никогда автоматически не меняет global project key. Это только evidence-backed гипотеза для будущего ambiguity layer.

### Реализовано и принято

- отдельный host-neutral `LocalKeyCenterAnalyzer`;
- `KeyCenterStatus`: `candidate`, `tonicized`, `established`, `modulationCandidate`;
- unresolved `ii–V` / `iiø–V` → candidate local center;
- `V→target` → confirmed temporary tonicization;
- `SubV→target` → confirmed temporary tonicization;
- полный local `ii–V–I` / `iiø–V–i` → established local center;
- полный local `ii–SubV–I` / `iiø–SubV–i` → established local center;
- remote centers могут находиться вне набора ступеней global key;
- `localHarmonic` хранит функцию current chord относительно local center;
- `localPattern` хранит pattern относительно local center;
- global-key cadence не создаёт redundant local center;
- `V–I–X`, где X поддерживает local center и конфликтует с global key, создаёт только `modulationCandidate`;
- отдельные evidence flags для candidate / tonicization / local cadence / modulation evidence;
- diagnostic UI показывает global и local interpretation раздельно;
- отдельный набор `SmartImproviserLocalKeyCenterTests`;
- Stage 1 regression не обнаружен.

## Версия 0.2d

```text
Build label: Smart Improviser 0.2d
CMake:      0.2.5
Artifact:   Smart-Improviser-0.2d-Windows
Package:    Smart Improviser.vst3
```

Финальный Windows Build #180 прошёл Configure / Build / Test / package / artifact upload успешно.

## Пройденные live-tests 0.2d

- F major: `Em7b5 → A7 → Dm`, current A7 → `D minor | local | established`;
- F major: `Em7b5 → A7`, current Em7b5 → candidate `D minor | temporary`;
- C major: `D7 → G` → temporary G major;
- C major: `Ab7 → G` → temporary G major through SubV;
- C major: `G#m7 → C#7 → F#maj7` → remote local F# major;
- global `Dm7 → G7 → Cmaj7` не создаёт отдельный local center;
- Stage 1 regression повторно проверен.

## Линия Stage 2

```text
0.2a — Harmonic Engine foundation          [ACCEPTED]
0.2a fix1 — Harmonic Engine diagnostics    [ACCEPTED]
0.2b — Pattern Recognizer                  [ACCEPTED]
0.2c — Tritone Substitution                [ACCEPTED]
0.2d — Local Key Center                    [ACCEPTED]
0.2e — Ambiguity / Confidence              [NEXT]
0.2f — Integration / musical validation
0.3  — Stage 2 complete
```

## Следующий checkpoint — 0.2e Ambiguity / Confidence

`0.2e` должен научить движок не делать преждевременный единственный вывод там, где один и тот же контекст допускает несколько музыкально правдоподобных трактовок.

План:

- `unique / ambiguous` interpretations;
- alternative interpretation candidates;
- borrowed/modal ambiguity;
- candidate local centers с confidence/evidence;
- разрешение конфликтов global pattern vs local pattern;
- ambiguous-context regression tests;
- диагностический вывод альтернатив и confidence для live-test.

## Что читать в новом чате Stage 2

1. `docs/CURRENT_STATE.md`;
2. `docs/STAGE_1_TO_STAGE_2_CONTRACT.md`;
3. `docs/CORE_DATA_MODEL_0.0b.md`;
4. `docs/PROJECT_CONTEXT.md`;
5. `docs/ARCHITECTURAL_DECISIONS.md`;
6. `docs/ROADMAP.md`;
7. `docs/VERSIONING.md`;
8. Issue #3 — Stage 2 — Harmonic Engine;
9. PR #18 — `0.2d — Local Key Center`.

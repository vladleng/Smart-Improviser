# Smart Improviser — Current State

> Планирование обновлено 2026-09-24: согласован workflow работы над песней, user content и постоянных tension-подсказок. 0.3a и 0.3b приняты; начата рабочая версия 0.3c; последняя принятая stable — 0.3.

> Короткая точка входа для нового чата или рабочей сессии. Подробная архитектура — в `PROJECT_CONTEXT.md`, этапы — в `ROADMAP.md`, правила версий — в `VERSIONING.md`, фактический прогресс — в GitHub Issues.

## Текущее состояние

- **Завершённые Stage:** Stage 0, Stage 1, Stage 2
- **Текущая стабильная версия:** `0.3`
- **Последний завершённый Stage:** Stage 2 — Harmonic Engine
- **Текущий Stage:** Stage 3 — Improvisation Engine
- **Рабочая линия:** `0.3a → 0.3x`
- **Итог Stage 3:** `0.4`
- **Stage 2 Issue:** #3 — закрыт стабильной `0.3`
- **Stage 3 Issue:** #4 — обновлён рабочий план `0.3a–0.3h`; 0.3a и 0.3b приняты; начат 0.3c

`0.3` — релизное закрытие Stage 2. Новая музыкальная логика относительно принятого `0.2f` не добавлялась: stable release фиксирует уже проверенное состояние Harmonic Engine.

## Архитектурная граница

Stage 1 contract остаётся закрытым. Harmonic Engine получает только host-neutral timeline context:

```text
previous chord
current chord
next chord
global key
```

Project key в DAW автоматически не меняется. Core не зависит от JUCE / ARA / Fender Studio Pro.

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

## Stable 0.3 — Stage 2 complete

В стабильную `0.3` входят все принятые checkpoints:

```text
0.2a — Harmonic Engine foundation          [ACCEPTED]
0.2a fix1 — Harmonic Engine diagnostics    [ACCEPTED]
0.2b — Pattern Recognizer                  [ACCEPTED]
0.2c — Tritone Substitution                [ACCEPTED]
0.2d — Local Key Center                    [ACCEPTED]
0.2e — Ambiguity / Confidence              [ACCEPTED]
0.2e fix1 — Enharmonic spelling            [ACCEPTED]
0.2f — Integration / musical validation    [ACCEPTED]
0.3  — Stage 2 complete                    [STABLE]
```

### Возможности Harmonic Engine

- basic harmonic functions;
- major `ii–V–I` на `ii / V / I`;
- minor `iiø–V–i` на `iiø / V / i`;
- `V–I`;
- `I–VI–ii–V`;
- secondary dominants и dominant chains;
- ordinary `V7` / `SubV7`;
- major/minor `ii–SubV–I`;
- applied SubV и guide-tone resolution;
- local key center: `candidate / tonicized / established / modulationCandidate`;
- remote local centers;
- global/local harmonic interpretations;
- borrowed/modal ambiguity;
- `unique / ambiguous` semantics и explicit primary interpretation;
- enharmonic-aware KeyCenter display через `rootFifths`;
- boundary false-positive guards;
- diagnostic UI для global/local/ambiguity/confidence.

### Integration validation

`0.2f` добавил отдельный regression target:

```text
SmartImproviserIntegrationValidationTests
```

Live-tested в Fender Studio Pro:

```text
Cmaj7 → A7 → Dm7 → G7 → Cmaj7
Cmaj7 → Fm7 → G7 → Cmaj7
Em7b5 → Eb7 → Dm → G7 → Cmaj7
C#7 → F#maj7 → Bmaj7
Dm7 → G7 → Abmaj7
```

Подтверждены переходы:

```text
Global → Local primary → Global
Unique → Ambiguous → Unique
Tonicized → Modulation candidate
```

Stage 1 / ARA regression не обнаружен.

## Версия 0.3

```text
Build label: Smart Improviser 0.3
CMake:      0.3.0
Artifact:   Smart-Improviser-0.3-Windows
Package:    Smart Improviser.vst3
Tests:      7 regression/integration targets
```

## Следующий этап — Stage 3 / Improvisation Engine

Stage 3 должен использовать готовый `HarmonicSituation` и выдавать музыкально применимый материал для импровизации:

- chord tones;
- guide tones;
- target notes;
- scales;
- harmonic concepts;
- resolution notes;
- базовые improvisation strategies.

Рабочая декомпозиция опубликована в [STAGE_3_PLAN.md](STAGE_3_PLAN.md) и Issue #4. Начальная методика согласована в [IMPROVISATION_METHOD.md](IMPROVISATION_METHOD.md); 0.3a и 0.3b приняты, следующий checkpoint — 0.3c.

Согласованная продуктовая цель: [PRODUCT_WORKFLOW.md](PRODUCT_WORKFLOW.md). Общая/личная библиотеки — Stage 5; сохраняемый Song workspace — Stage 7; быстрый ввод нот/TAB и ручной конструктор — Stage 8; Planner — Stage 10. Смена tension не переписывает сохранённые идеи.

## Что читать при переходе к Stage 3

1. `docs/CURRENT_STATE.md`;
2. `docs/ROADMAP.md`;
3. `docs/STAGE_1_TO_STAGE_2_CONTRACT.md`;
4. `docs/PROJECT_CONTEXT.md`;
5. `docs/ARCHITECTURAL_DECISIONS.md`;
6. `docs/VERSIONING.md`;
7. Issue #3 — история завершённого Stage 2.

8. `docs/PRODUCT_WORKFLOW.md` — согласованный пользовательский сценарий;
9. `docs/STAGE_3_PLAN.md` — рабочий план следующего этапа и открытые решения.

## Принятый checkpoint 0.3a

Реализована основа Improvisation Engine: одна стратегия по explicit chord tones, сохранение гармонического контекста и target, отдельные source/thinking/resolution поля, deterministic rule ID/version и priority. Диагностика справа; анализ вне paint. Локально 8 C++ test targets проходят. Windows Build #231 — success. Влад принял живой тест 2026-09-24, PR #23 слит в main.

[Чек-лист живой проверки](STAGE_3_0.3a_LIVE_TEST.md). Гаммы, модели Бойко и выбор T1–T3 ещё не реализованы.

## Принятый checkpoint 0.3b

Подготовлены guide tones (только присутствующие 3/7), characteristic tones, реальные звуки следующего аккорда и показ движений ResolutionTarget Stage 2. Для неподтверждённого перехода предлагаются отдельные optional melodic connections к ближайшему root/guide следующего аккорда; они не подтверждают гармоническую функцию.

8 локальных C++ test targets проходят. Windows Build #234 — success; Влад принял живой тест, PR #24 слит в main. [Чек-лист 0.3b](STAGE_3_0.3b_LIVE_TEST.md). Материал пока отображается как pitch classes. Stable остаётся 0.3.

## Рабочий checkpoint 0.3c

Добавлен ограниченный диатонический каталог: режимы принятого major/minor center и Mixolydian для ordinary dominant seventh с подтверждённой major target. Сначала проверяется совместимость со всеми explicit chord tones/degrees. Unresolved primary, minor-target dominant, SubV, цепи и неподдерживаемые случаи сохраняют chord-tone foundation с объяснением отсутствия гаммы.

Scale source хранит ступени, root/spelling и ссылку на interpretation; source не меняет tonal center. Опоры, characteristic tones и resolution 0.3b сохраняются. Новая гамма выводится с корректным enharmonic spelling; прежние списки опор пока pitch classes. Tension policy не реализуется.

9 локальных C++ test targets проходят; Windows CI отслеживается в PR. Живой тест 0.3c ещё не принят. [Чек-лист](STAGE_3_0.3c_LIVE_TEST.md).

# Stage 3 / 0.3g — Explanation / usable output

## Цель

После принятого `0.3f fix4` Harmony/Improvisation Core уже знает текущий аккорд, global/local/modal interpretations, patterns, targets/resolution, rootless/implied reading и incomplete cadence. `0.3g` не добавляет новую гармоническую теорию: он превращает уже рассчитанные данные в короткую практическую цепочку для музыканта.

```text
идея → source → важные ноты → target/resolution → почему
```

## Архитектурное правило

Explanation строится **только из готового `ImprovisationResult` / `HarmonicSituation`**. Никакого повторного анализа harmony/history в UI.

Core предоставляет host-neutral presentation model. UI отвечает за локализацию, layout и цвет. Текущий промежуточный diagnostic formatter сериализует те же готовые data objects для существующей текстовой панели и не является новым harmonic analyzer.

## Core contract

0.3g использует `ExplanationResult` / `ExplanationItem` / `ExplanationEvidenceItem` и `ExplanationContextLayer`.

Для каждого визуально уникального музыкального материала хранятся:

- исходные strategy indices;
- provenance по interpretation indices;
- idea / conditions / usage hint;
- source material;
- actual chord / thinking structure;
- важные ноты (guide + characteristic, fallback — source notes);
- next target / target notes;
- confirmed resolution;
- Why?-evidence.

Визуально одинаковый material из нескольких interpretations схлопывается в один item, но `interpretationIndices` сохраняются. Enharmonic spelling является частью визуальной идентичности: например, Db- и C#-материал не склеиваются только из-за совпадения pitch class.

`ExplanationContextLayer` отдельно хранит global / local / modal уровни с их собственными center, harmonic function и evidence. Это presentation data, а не новый harmonic inference.

## Why?-evidence

Поддерживаемые причины:

- top-level `PatternContext`;
- nested pattern;
- confirmed `ResolutionTarget`;
- `ImpliedDominantReading`;
- `IncompleteCadence` / missing tonic;
- фактическое продолжение, противоречащее ожидаемой tonic;
- unresolved multiple interpretations.

Presentation-state:

- `confirmed` — подтверждённый pattern/resolution;
- `missing` — отсутствующий member полного шаблона;
- `implied` — подразумеваемая функция/root или provisional evidence;
- `contradicted` — известный факт противоречит гипотезе;
- `ambiguous` — несколько допустимых interpretations без hidden winner.

Цветовой контракт UI остаётся принятым:

- серый = missing / implicit;
- янтарный = implied / provisional;
- красный = contradicted.

## Presentation

В карточке «Текущий контекст» уровни должны быть названы явно:

- глобальная функция;
- локальный центр;
- локальная функция — только когда она реально рассчитана;
- modal interpretation — как отдельная трактовка.

Это закрывает известный кейс `Gm7`: global function в C и local F-major center не должны выглядеть одной и той же функцией.

Раздел «Материал» уже получает промежуточный diagnostic block из `ExplanationResult`: context layers, idea, source, important notes, target/resolution и Why?-evidence. Подробные исходники/ноты и diagnostics остаются в существующих разделах. Финальная локализация/layout и частичная цветовая разметка остаются presentation-задачей 0.3g.

## Scope 0.3g

- [x] Создать host-neutral Explanation API поверх готового `ImprovisationResult`.
- [x] Схлопывать одинаковый material без потери interpretation provenance.
- [x] Не склеивать визуально различающийся enharmonic spelling.
- [x] Включить PatternContext / nested patterns / resolution / implied dominant / incomplete cadence / contradiction / ambiguity в Why?-evidence model.
- [x] Добавить явные `global / local / modal` context layers в Explanation data.
- [x] Покрыть Explanation API regression-тестами.
- [x] Подключить ExplanationResult к существующей текстовой панели «Материал» без повторного анализа.
- [x] Добавить промежуточный output `идея → source → важные ноты → target/resolution → почему`.
- [x] Явно разделить global/local/modal уровень функции в верхнем summary UI.
- [x] Реализовать настоящие цвета missing/implied/contradicted в presentation.
- [x] Обновить build label и Windows artifact на `0.3g`.
- [x] Актуальный regression suite после presentation commit green (Build #346: 15/15).
- [ ] Live-test Studio Pro.
- [ ] Acceptance пользователя.

## Regression matrix

Проверяется:

1. ordinary major `ii–V–I` — confirmed resolution в Why?;
2. global context layer;
3. одинаковый material из двух interpretations схлопывается, provenance сохраняется;
4. Db/C# spelling не склеивается как «визуально одинаковый» material;
5. Gm7-контракт: global C и local F передаются отдельными context layers;
6. unresolved ambiguity — отдельный `ambiguous` evidence;
7. `D7/A → Abdim` — rootless/implied dominant evidence типа `implied`;
8. `Fm7 → Bb7 → Em7` — missing I отдельно от contradicted actual continuation;
9. invalid result не создаёт ложное explanation;
10. все прежние Stage 1/2/0.3f regression targets должны оставаться green.
11. `Em7 → A7` без известного продолжения — только кандидат `ii–V–I` в D: ожидаемая I показана янтарным как неподтверждённая, локальный центр отмечен кандидатом; это не `IncompleteCadence` с отсутствующей I, пока неизвестен следующий аккорд.

## CI

- Windows Build #338 — success для первого Explanation API/test commit-set.
- После добавления context layers и Material output выполнялись промежуточные Windows gates.
- Windows Build #345 — ошибка компиляции: `ExplanationText.h` не видел `explainImprovisation`; зависимость исправлена.
- Windows Build #346 — success: компиляция, 15/15 тестовых целей, VST3 artifact `Smart-Improviser-0.3g-Windows`.
- После live-наблюдения `Fm7 → Bb7 → Em7` и `Em7 → A7` уточнён верхний summary: известное противоречие даёт серую отсутствующую Eb, неизвестное продолжение — янтарную ожидаемую D. Локальный тест `PatternContextTests` пройден; Windows gate для этого UI-изменения ожидается.

## Gate

```text
Core explanation tests
        ↓
all regression targets green
        ↓
Smart-Improviser-0.3g-Windows
        ↓
Studio Pro live-test
        ↓
user acceptance
```

Tension T1/T2/T3 остаётся Stage 4 и в 0.3g автоматически не назначается.

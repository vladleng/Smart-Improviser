# ADR-019 — Confirmed harmonic event сохраняется как PatternContext

**Статус:** принято, 2026-09-25; реализация — `0.3f fix1`.

## Контекст

Stage 1 передаёт в Core только `previous / current / next / global key`. Этого достаточно для распознавания многих локальных паттернов, но на resolution chord earlier predominant уже может выпасть из snapshot.

Пример:

```text
F major: Gm7 → C7 → Fmaj7
          ii     V      I
```

На `C7` полный `ii–V–I` уже подтверждён. На `Fmaj7` raw Stage 1 snapshot содержит только `C7→Fmaj7`, поэтому stateless Stage 2 analysis деградирует до `V–I`, хотя музыкальный event ещё не завершён в пользовательском смысле.

Также длинная кадансовая цепь:

```text
Am7 → D7 → Gm7 → C7 → Fmaj7
 iii    VI7    ii     V      I
```

должна иметь единый top-level context, сохраняя локальные связи `V/ii→ii` и `ii–V–I` как вложенное evidence.

## Решение

Core вводит host-neutral `PatternContext`, описывающий уже распознанный harmonic event:

```text
pattern type
center / harmonic interpretation
member count
current member index
status: candidate / confirmed / completed
start/resolution identity or position
nested/subpattern evidence
confidence/evidence
```

Чтобы PatternContext переживал границу `previous/current/next`, **runtime-cache предыдущего результата не используется**. Вместо него Stage 3 получает отдельный bounded `PatternTimelineWindow` (до 9 chord events), который адаптер каждый раз заново строит из текущего host timeline. Harmonic Engine детерминированно реконструирует PatternContext из этого окна.

Таким образом:

```text
Stage 1 contract:
previous / current / next / global key

отдельный Stage 3 input:
bounded PatternTimelineWindow
        ↓
PatternContext
```

`PatternTimelineWindow` не является частью `TimelineHarmonicSnapshot` и не меняет принятый Stage 1 → Stage 2 контракт.

Generic `V→I/i` остаётся fallback, если отдельного подтверждённого PatternContext восстановить нельзя.

## Почему reconstruction вместо runtime memory

Такое решение автоматически делает seek/edit/reopen безопасными:

- прямой seek на `Fmaj7` может восстановить `ii–V–I • 3/3` без предварительного playback через `Gm7` и `C7`;
- переход назад по timeline даёт тот же результат, что и движение вперёд;
- chord edit меняет reconstruction на следующем analysis pass;
- reopen не требует сериализовать ephemeral harmonic-analysis state;
- одинаковый timeline + position дают одинаковый результат.

## Invariants

- Stage 1 contract **не расширяется** до `previous2 / previous3 / ...`.
- `PatternTimelineWindow` bounded (`kMaxPatternWindowChords = 9`) и предназначен только для Stage 3 pattern reconstruction.
- PatternContext не является общей историей аккордов.
- Нет скрытого runtime-cache, который мог бы устареть после seek/chord edit/reopen.
- Global/local/modal interpretation semantics сохраняются.
- Confidence analysis и recommendation priority остаются раздельными.
- Core остаётся независимым от JUCE/ARA; ARA adapter только формирует host-neutral window.

## Иерархия

Модель допускает:

```text
top-level pattern
nested subpatterns/evidence
candidate next event
completed event
```

Один chord может завершать предыдущий pattern и одновременно давать evidence следующего события.

Для первого scope `0.3f fix1` top-level extended pattern ограничен случаем:

```text
F major: iii–VI7–ii–V–I
```

с пользовательским отображением **«Расширенный каданс iii–VI7–ii–V–I»**. Широкий graph любых длинных прогрессий не входит в fix1.

## Последствия

- `ii–V–I`, `iiø–V–i`, `iv–V–i` корректно показывают `3/3` на resolution chord, когда bounded reconstruction подтверждает весь event.
- Если reconstruction не содержит достаточного evidence, сохраняется безопасный `V→I/i` из принятого `0.3f`.
- `iii–VI7–ii–V–I` становится одним top-level pattern `1/5 … 5/5`, а `V/ii→ii` и `ii–V–I` сохраняются как nested evidence.
- `0.3g Why?` сможет использовать готовое top-level/nested evidence без повторного анализа истории в UI.
- Regression должен отдельно проверять mapper/reconstruction и Core pattern semantics.

Подробный scope: [STAGE_3_0.3f_FIX1_PLAN.md](STAGE_3_0.3f_FIX1_PLAN.md).

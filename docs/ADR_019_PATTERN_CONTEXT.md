# ADR-019 — Confirmed harmonic event сохраняется как PatternContext

**Статус:** принято на уровне направления, 2026-09-25; реализация — `0.3f fix1`.

## Контекст

Stage 1 передаёт в Core только `previous / current / next / global key`. Этого достаточно для распознавания многих локальных паттернов, но на resolution chord earlier predominant уже может выпасть из snapshot.

Пример:

```text
F major: Gm7 → C7 → Fmaj7
          ii     V      I
```

На `C7` полный `ii–V–I` уже подтверждён. На `Fmaj7` raw snapshot содержит только `C7→Fmaj7`, поэтому stateless analysis деградирует до `V–I`, хотя музыкальный event ещё не завершён в пользовательском смысле.

Также длинная кадансовая цепь:

```text
Am7 → D7 → Gm7 → C7 → Fmaj7
 iii    VI7    ii     V      I
```

должна иметь единый top-level context, сохраняя локальные связи `V/ii→ii` и `ii–V–I` как вложенное evidence.

## Решение

Core вводит ограниченный host-neutral state распознанного harmonic event — `PatternContext` / `RecognizedPatternInstance`.

Минимально он хранит:

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

PatternContext разрешается переносить на следующий chord только если event был подтверждён и текущий chord соответствует ожидаемому member/resolution.

Generic `V→I/i` остаётся fallback при отсутствии carried confirmed context.

## Invariants

- Stage 1 contract **не расширяется** до `previous2 / previous3 / ...`.
- PatternContext не является общей историей аккордов.
- State имеет ограниченную lifetime и должен сбрасываться после completion/выхода из релевантного участка.
- Seek, chord edits и reopen пересчитывают context из timeline/evidence; stale runtime memory запрещён.
- Global/local/modal interpretation semantics сохраняются.
- Confidence analysis и recommendation priority остаются раздельными.
- Core остаётся независимым от JUCE/ARA.

## Иерархия

Модель допускает:

```text
top-level pattern
nested subpatterns/evidence
candidate next event
completed carried event
```

Один chord может завершать предыдущий pattern и одновременно давать evidence следующего события.

Для первого scope `0.3f fix1` top-level extended pattern ограничен случаем:

```text
F major: iii–VI7–ii–V–I
```

с пользовательским отображением **«Расширенный каданс iii–VI7–ii–V–I»**. Широкий graph любых длинных прогрессий не входит в fix1.

## Последствия

- `ii–V–I`, `iiø–V–i`, `iv–V–i` могут корректно показывать `3/3` на resolution chord при наличии подтверждённого carried context.
- `0.3g Why?` сможет использовать уже готовое top-level/nested evidence без повторного анализа истории в UI.
- Для pattern state обязательны regression tests на seek/edit/reopen.

Подробный scope: [STAGE_3_0.3f_FIX1_PLAN.md](STAGE_3_0.3f_FIX1_PLAN.md).

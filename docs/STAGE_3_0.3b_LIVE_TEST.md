# Stage 3 / 0.3b — Structural tones / targets / resolution

Статус: реализация подготовлена, живой тест ожидается. 0.3a принят Владом; Stage 3 Issue #4 остаётся открытым.

## Новое

- Guide tones — присутствующие терции и септимы; sus/triads не получают отсутствующие ноты. Explicit #9 не считается малой терцией, explicit diminished seventh отличается от sixth.
- Characteristic tones — остальные явно заданные звуки, кроме корня и чистой квинты: sus, изменённая квинта, надстройки/альтерации. Это описание аккорда, не оценка tensions.
- Target notes — реальные звуки следующего аккорда относительно его корня.
- Confirmed resolution — уже подтверждённые движения Stage 2, без повторного анализа.
- Suggested connections — отдельные optional мелодические переходы к ближайшим root/guide следующего аккорда, если подтверждённого разрешения нет. При равной дистанции первым выбирается звук в порядке интервалов от корня. Удержание общего тона допустимо. Эти предложения не меняют harmonic evidence.
- Источники-гаммы и T1–T3 selection остаются следующими checkpoints/Stage.

## Живой тест

Установить `Smart-Improviser-0.3b-Windows`, заменить `Smart Improviser.vst3`; заголовок 0.3b. Проект C major.

| Аккорды | Ожидаемый результат на первом аккорде |
|---|---|
| G7 → Cmaj7 | Guides B F; targets C E G B; confirmed B → C, F → E |
| G7 → Cm | Guides B F; targets C Eb G; confirmed B → C, F → Eb |
| A7 → Dm7 | Guides Db G (Db = C#); targets D F A C; confirmed Db → D, G → F |
| Db7 → Cmaj7 | Guides F B (B = Cb); confirmed F → E, B → C |
| G7sus4 → Cmaj7 | Guides только F; characteristic C; отсутствующая B не добавляется |
| C → Dm7 | Guides только E; нет добавленной септимы; targets D F A C; optional E → F |
| G7b9 → Cmaj7 | Characteristic Ab сохранён; guides B F |
| G7 → Abmaj7 | Targets Ab C Eb G; suggested connections B → C, F → G; нет confirmed resolution |
| G7 без следующего аккорда | No next chord, нет движений и старых targets |

- [ ] Мажорное и минорное разрешения различаются без смены глобальной тональности проекта.
- [ ] Secondary/SubV cases соответствуют таблице.
- [ ] Sus/triad/alteration cases не теряют и не добавляют звуки.
- [ ] Confirmed и suggested явно различимы.
- [ ] PLAY/STOP/seek, редактирование цели, исчезновение контекста и reopen обновляют карточку корректно.
- [ ] Карточка читаема, Stage 1–2 diagnostics без регрессии.

Pitch-class spelling временно сохраняется из 0.3a (Db вместо C#, B вместо Cb). Это не смена функции нот. После принятия 0.3b — 0.3c.

Автоматически: 8 C++ test targets, дополнены проверки guide/characteristic roles, major/minor/SubV moves, target-relative notes, sus/triads/#9/dim7/m6, optional connections, отсутствующего next и 12 тональностей. Локальная проверка — g++; Windows VST3 — CI.

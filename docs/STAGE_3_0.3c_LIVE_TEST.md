# Stage 3 / 0.3c — Diatonic / modal sources

Статус: принят Владом 2026-09-25. Windows Build #237 success; PR #25 merged. Переход к 0.3d разрешён.

## Объём

Сохраняется chord-tone foundation 0.3b. К нему добавляется не более одной scale strategy: её source, relative degrees/spelling, interpretation evidence, guide/target notes и resolution. Приоритет foundation 100, scale 50; это не confidence и не Tension Level. UI объединяет их в одной карточке.

Обычная доминанта-септаккорд с подтверждённой мажорной целью получает базовый Mixolydian, только если все явные звуки и ступени совместимы. Minor-target dominant не получает Mixo автоматически: пока опоры и minor resolution, melodic-minor модели — 0.3d. SubV, цепи, unresolved dominant и sus требуют отдельных правил. Harmonic minor остаётся отложенным расширением.

Для остальных поддерживаемых аккордов источник — семиступенный режим выбранного Stage 2 major/minor center: Ionian, Dorian, Phrygian, Lydian, Aeolian, Locrian. Это техническая расшифровка диатонического контекста, а не утверждение, что Бойко предлагает все эти режимы как отдельные авторские методы. Основной методический источник сохраняется; специальные melodic-minor и altered модели идут следующим checkpoint.

Правила не подбирают произвольную гамму по одному chord quality. Нет primary interpretation — нет уверенного scale source. Неподдерживаемый custom center или конфликт explicit alteration/degree дают объяснение и сохраняют опоры. Гамма не меняет global/local center. Все её ноты не объявляются одинаково устойчивыми; natural 4th против major 3rd явно отмечается как проходящая.

Новый список гаммы использует ступенное написание (например E#, B#, Cb), source rootFifths и ступени 1–7 относительно текущего корня. Прежние списки chord/guide/target notes пока сохраняют pitch-class написание 0.3b.

## Проверка в Fender Studio Pro

Artifact `Smart-Improviser-0.3c-Windows`; package `Smart Improviser.vst3`; заголовок 0.3c.

Проект C major:

| Сценарий | Ожидаемый SCALE SOURCE |
|---|---|
| Dm7 → G7 → Cmaj7, на Dm7 | D Dorian: D E F G A B C |
| Тот же оборот, на G7 | G Mixolydian: G A B C D E F; natural 4th passing; B → C, F → E сохранены |
| На Cmaj7 при принятой C-major трактовке | C Ionian: C D E F G A B |
| G7 → Cm, global key остаётся C major | Нет Mixo; сообщение о minor-target source следующего checkpoint; B → C, F → Eb сохранены |
| D7 → Gmaj7 | D Mixolydian: D E F# G A B C; функция и цель Stage 2 сохраняются |
| G7b9 → Cmaj7 | Нет несовместимого Mixo; explicit Ab остаётся в опорах |
| G7sus4 → Cmaj7 | Нет автоматически добавленной scale third; только опоры/targets |
| Db7 → Cmaj7 | Сообщение о SubV source следующего checkpoint; разрешения сохранены |
| G7 → C7 / G7 без следующего аккорда | Нет автоматического Mixo для неопределённого/иного target |

Дополнительные проверки:

- [ ] Локальный центр из Stage 2 влияет на источник, глобальная тональность проекта не меняется. Если primary unresolved, показывается объяснение вместо уверенной гаммы.
- [ ] В C# major на C#maj7: C# D# E# F# G# A# B#.
- [ ] В Gb major на Gbmaj7: Gb Ab Bb Cb Db Eb F.
- [ ] PLAY/STOP/seek и Chord/Key edits обновляют источник вместе с контекстом.
- [ ] При потере валидного контекста старая гамма исчезает; reopen восстанавливает результат.
- [ ] Карточка читаема; chord/guide/characteristic/target notes и confirmed/suggested движения 0.3b работают.

Автоматически: 9 C++ test targets. Новый target проверяет rotation modes, real ii–V, minor/SubV/chain/alteration guards, выбранный local center, unresolved primary, explicit degree conflict, 12 тональностей, deterministic output и enharmonic spelling включая двойные альтерации. Локально g++; Windows VST3 — CI. После принятия 0.3c — 0.3d.

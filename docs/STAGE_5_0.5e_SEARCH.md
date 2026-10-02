# 0.5e — Context-aware library search
База: 0.5d принята Владом 2026-10-02, PR #62 merged 9a7f3f3.
Stable остаётся 0.5. Первый common vocabulary — 0.5f.

## Контракт
searchLibrary получает detached records и уже рассчитанные ImprovisationResult для slots.
Порядок: validation/metadata → готовность Phrase → assessPhrase / assessPhraseAgainstCurve →
допуск по tension → deterministic ranking только допущенных фраз.
Idea не превращается в Phrase. Значение T1 по умолчанию не считается назначением.
Поиск не меняет content, notes, register, rhythm, lineage или shared manual tensions.

Фильтры: текст (UTF-8, регистр учитывается), точный tag/concept, роль, оборот,
позиция в выбранном обороте и функция текущего контекста.
Функция проверяется по global/local и сохранённым interpretations; совпадение одной трактовки
не объявляет её единственно верной. Позиция учитывает top-level/nested pattern.
Unknown metadata/context не выдаётся за mismatch; причины показаны отдельно.

Scalar tension использует описательный профиль через принятый matcher.
Optional desired curve заменяет scalar request: границы абсолютные в beats песни,
placementStartBeat явный. Invalid/unknown/mismatched profiles не ранжируются.
Ranking: конкретность гармонических требований, затем domain/ID/revision; это не confidence.
Actual next chord и подтверждённая/предполагаемая цель остаются раздельными.

## UI и границы
«Подбор фраз» на основном экране открывает окно результатов с фильтрами и причинами.
«Подобрать» снимает новый снимок cursor/context; поиск не опрашивает файл и не работает в audio callback.
Adapter использует готовые Stage 2/3 анализаторы для максимум 16 slots / 64 beats.
Начало фразы — текущий курсор; slot 1 — текущий аккорд. Нужны известные границы.
Последний аккорд без следующей известной границы не получает выдуманную длительность.
Future truncation даёт insufficientContext. Это не Song placement/persistence или транспозиция.

Справа показаны сохранённые ступени/ритм, source versions/provenance, actual next и статус цели.
Нотный viewer готовых фраз и Song workspace остаются Stage 7, полный редактор — Stage 8.
Интегрирован импорт .silibrary через окно «Библиотека»; конфликтующие IDs не перезаписываются.

## Проверочный файл
Windows artifact содержит Search-check.silibrary, созданный regression executable тем же codec.
Импорт добровольный, ничего не добавляется автоматически. Это собственные трёхнотные
chord-anchor fixtures для проверки ПО, а не первый common vocabulary или рекомендации tension.
Тестовые T1/T2 присвоены явно исключительно для проверки фильтра.
Записи имеют fixed namespaced IDs; повторный импорт сообщает конфликт.

## Live checklist
1. Импортировать Search-check.silibrary через «Библиотека» → «Импорт библиотеки».
2. На G7 перед Cmaj7, курсор в начале G7, до следующего аккорда не менее 1.5 beats:
   All — 3 подходят, T1 — 1, T2 — 1; запись без метки сообщает unknown при T1/T2.
   Минорное требование не проходит гармонию; текстовая Idea остаётся наброском.
3. На Dm7 эти G7-фразы не допускаются. Смена фильтра не переписывает материал.
4. Проверить text/tag/concept/role и пустой результат; search-check — общий tag fixtures.
5. Без текущего контекста / на последнем аккорде без известной границы — нет скрытого победителя.
6. Кривая: songStart,songEnd,T, spans через точку с запятой; это абсолютные beats песни.
7. Перезапуск Studio Pro: импорт и собственные идеи сохранены, прошлые context/viewer/tension сценарии работают.

Regression: harmonic-first, All/unclassified, scalar/curve, metadata, missing context, obsolete version,
determinism, byte-identical archive before/after search и bounded timeline mapping.
Windows Build #441 на code 801f2a4 — success: VST3, 33/33 CTest и создание Search-check.silibrary.
https://github.com/vladleng/Smart-Improviser/actions/runs/36952524226
Artifact: Smart-Improviser-0.5e-Windows. Studio Pro acceptance ожидается.

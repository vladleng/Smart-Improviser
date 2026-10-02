#include "core/analysis/LibrarySearch.h"
#include <algorithm>
namespace smartimproviser::harmony
{
namespace {
bool contains(const std::string& text,const std::string& token) {return text.find(token)!=std::string::npos;}
const char* reason(PhraseMatchReason r) {
    switch(r) {
    case PhraseMatchReason::matched:return "Гармонические требования выполнены.";
    case PhraseMatchReason::sourceVersion:return "Версия источника требует повторной проверки.";
    case PhraseMatchReason::ambiguousSource:return "Есть несколько трактовок источника; совпадение не подтверждено.";
    case PhraseMatchReason::sourceUnavailable:return "Нужный источник отсутствует или не указан.";
    case PhraseMatchReason::chordMismatch:return "Тип фактического аккорда не соответствует фразе.";
    case PhraseMatchReason::patternMismatch:return "Не подтверждён требуемый гармонический оборот.";
    case PhraseMatchReason::destinationMissing:return "Неизвестна цель разрешения.";
    case PhraseMatchReason::destinationMismatch:return "Мажорная/минорная цель не совпадает.";
    case PhraseMatchReason::destinationUnconfirmed:return "Цель только предполагается; подтверждения нет.";
    case PhraseMatchReason::unsupportedNote:return "Нота не выполняет указанную роль в фактическом аккорде/источнике.";
    case PhraseMatchReason::invalidApproach:case PhraseMatchReason::missingApproach:return "Подход/enclosure не подтверждён полностью.";
    case PhraseMatchReason::unsupportedOutside:return "Для outside-нот нет подтверждённого правила.";
    case PhraseMatchReason::invalidTiming:return "Ритм не помещается в известные границы аккордов.";
    case PhraseMatchReason::missingContext:return "Не хватает известных аккордов или их границ.";
    case PhraseMatchReason::tensionUnclassified:return "Tension фразы не оценена полностью.";
    case PhraseMatchReason::tensionMismatch:return "Tension фразы отличается от запроса.";
    case PhraseMatchReason::tensionProfileInvalid:return "Профиль tension некорректен.";
    default:return "Музыкальные данные фразы некорректны.";
    }
}
LibrarySearchState contextFilter(const LibrarySearchQuery& q,std::vector<std::string>& why) {
    if(!q.function && !q.patternPosition)return LibrarySearchState::eligible;
    if(q.match.slots.empty() || !q.match.slots.front().material.valid) {
        why.push_back("Контекст для фильтра функции/позиции отсутствует.");return LibrarySearchState::insufficientContext;
    }
    const auto& s=q.match.slots.front().material.context;
    if(q.function) {
        bool known=false,matched=false;
        const auto examine=[&](const HarmonicAnalysis& h) {if(h.valid && h.effectiveFunction!=HarmonicFunction::undefined){known=true;matched=matched || h.effectiveFunction==*q.function;}};
        examine(s.harmonic);examine(s.localHarmonic);
        for(std::size_t i=0;i<s.interpretationCount && i<s.interpretations.size();++i)
            if(s.interpretations[i].valid)examine(s.interpretations[i].harmonic);
        if(!matched){why.push_back(known?"Функция текущего аккорда не совпадает с фильтром.":"Функция текущего аккорда неизвестна.");
            return known?LibrarySearchState::metadataMismatch:LibrarySearchState::insufficientContext;}
        why.push_back("Функция найдена в сохранённых трактовках контекста; неоднозначность не устраняется.");
    }
    if(q.patternPosition) {
        if(!q.pattern || *q.patternPosition<0) {why.push_back("Для позиции выберите конкретный оборот.");return LibrarySearchState::insufficientMetadata;}
        bool known=false,matched=false;
        const auto examine=[&](const HarmonicPattern& p) {if(p.type==*q.pattern && p.recognized()){
            known=true;matched=matched || p.positionIndex==*q.patternPosition;}};
        examine(s.pattern);examine(s.localPattern);examine(s.patternContext.topLevel);
        for(std::size_t i=0;i<s.patternContext.nestedPatternCount && i<s.patternContext.nestedPatterns.size();++i)examine(s.patternContext.nestedPatterns[i]);
        if(!matched) {why.push_back(known?"Позиция в обороте отличается.":"Позиция в нужном обороте неизвестна.");
            return known?LibrarySearchState::metadataMismatch:LibrarySearchState::insufficientContext;}
    }
    return LibrarySearchState::eligible;
}
}
LibrarySearchResult searchLibrary(const std::vector<LibraryRecord>& records,const LibrarySearchQuery& q)
{
    LibrarySearchResult out;
    for(const auto& record:records) {
        LibrarySearchEntry item;item.record=record;item.validation=validateLibraryRecord(record);
        const auto* phrase=std::get_if<Phrase>(&record.content);const auto* idea=std::get_if<Idea>(&record.content);
        const auto fail=[&](LibrarySearchState state,const char* why){item.state=state;item.explanations.push_back(why);};
        if(!item.validation.structurallyValid)fail(LibrarySearchState::invalidRecord,"Структура записи некорректна.");
        else {
            bool metadata=true;
            if(!q.text.empty()) {
                bool found=contains(record.name,q.text)||contains(record.explanation,q.text)||contains(record.source.author,q.text)||contains(record.source.source,q.text);
                if(idea)found=found||contains(idea->text,q.text)||contains(idea->harmonyNotes,q.text)||contains(idea->rhythmNotes,q.text);
                for(const auto& tag:record.tags)found=found||contains(tag,q.text);
                if(!found){fail(LibrarySearchState::metadataMismatch,"Текст не найден в записи.");metadata=false;}
            }
            if(metadata && !q.tag.empty() && std::find(record.tags.begin(),record.tags.end(),q.tag)==record.tags.end()) {
                fail(record.tags.empty()?LibrarySearchState::insufficientMetadata:LibrarySearchState::metadataMismatch,"Тег отсутствует в записи.");metadata=false;
            }
            const auto& concepts=phrase?phrase->conceptRuleIds:idea->conceptRuleIds;
            if(metadata && !q.concept.empty() && std::find(concepts.begin(),concepts.end(),q.concept)==concepts.end()){
                fail(concepts.empty()?LibrarySearchState::insufficientMetadata:LibrarySearchState::metadataMismatch,"Concept не указан или отличается.");metadata=false;
            }
            if(metadata && q.role && (!phrase || phrase->role!=*q.role)){
                fail(!phrase || phrase->role==PhraseRole::undefined?LibrarySearchState::insufficientMetadata:LibrarySearchState::metadataMismatch,"Роль фразы не указана или отличается.");metadata=false;
            }
            auto pattern=phrase?phrase->harmonicPattern:idea->harmonicPattern.value_or(HarmonicPatternType::undefined);
            if(metadata && q.pattern && pattern!=*q.pattern){
                fail(pattern==HarmonicPatternType::undefined || pattern==HarmonicPatternType::none?LibrarySearchState::insufficientMetadata:LibrarySearchState::metadataMismatch,"Оборот в записи не указан или отличается.");metadata=false;
            }
            if(metadata) {
                item.state=contextFilter(q,item.explanations);
                if(item.state==LibrarySearchState::eligible) {
                    if(!phrase || !item.validation.readyForSearch) {
                        fail(LibrarySearchState::draft,"Набросок сохранён, но готовность для гармонического поиска не подтверждена.");
                        for(const auto& d:item.validation.diagnostics)if(d.scope==LibraryValidationScope::searchReadiness)
                            item.explanations.push_back(d.field+": "+d.explanation);
                    } else {
                        if(q.curve) {
                            item.curveAssessment=assessPhraseAgainstCurve(*phrase,q.match.slots,*q.curve,q.placementStartBeat);
                            item.assessment=item.curveAssessment->harmonicMatch;
                        } else item.assessment=assessPhrase(*phrase,q.match);
                        if(item.assessment.harmonic==PhraseCompatibility::incompatible)item.state=LibrarySearchState::harmonicMismatch;
                        else if(item.assessment.harmonic==PhraseCompatibility::insufficientContext)item.state=LibrarySearchState::insufficientContext;
                        else if(q.curve) {
                            switch(item.curveAssessment->tension){
                            case TensionCurveMatch::matches:case TensionCurveMatch::unconstrained:item.state=LibrarySearchState::eligible;break;
                            case TensionCurveMatch::unclassified:item.state=LibrarySearchState::tensionUnknown;break;
                            case TensionCurveMatch::differentLevel:item.state=LibrarySearchState::tensionMismatch;break;
                            default:item.state=LibrarySearchState::invalidTension;break;
                            }
                            item.explanations.push_back(item.eligible()?"Желаемая кривая допускает фразу.":"Кривая tension не подтверждена или не совпадает.");
                        } else {
                            switch(item.assessment.tension) {
                            case PhraseTensionMatch::any:case PhraseTensionMatch::matches:item.state=LibrarySearchState::eligible;break;
                            case PhraseTensionMatch::unclassified:item.state=LibrarySearchState::tensionUnknown;break;
                            case PhraseTensionMatch::differentLevel:item.state=LibrarySearchState::tensionMismatch;break;
                            default:item.state=LibrarySearchState::invalidTension;break;
                            }
                        }
                        for(const auto& d:item.assessment.diagnostics)
                            item.explanations.push_back(std::string(reason(d.reason))+(d.chordIndex>=0?" Аккорд "+std::to_string(d.chordIndex+1)+".":""));
                        if(item.eligible()) {
                            item.specificity=static_cast<int>(phrase->harmonicRequirements.size())+(phrase->harmonicPattern!=HarmonicPatternType::undefined && phrase->harmonicPattern!=HarmonicPatternType::none?1:0);
                            ++out.eligibleCount;
                        }
                    }
                }
            }
        }
        out.entries.push_back(std::move(item));
    }
    std::sort(out.entries.begin(),out.entries.end(),[](const auto& a,const auto& b) {
        if(a.eligible()!=b.eligible())return a.eligible();
        if(a.eligible() && a.specificity!=b.specificity)return a.specificity>b.specificity;
        if(a.record.domain!=b.record.domain)return a.record.domain<b.record.domain;
        if(a.record.id!=b.record.id)return a.record.id<b.record.id;
        return a.record.revision<b.record.revision;
    });
    out.explanation=out.eligibleCount?"Найдены допустимые фразы. Порядок отражает конкретность требований, а не уверенность гармонического анализа."
        :records.empty()?"Библиотека пуста. Сохраните идею или импортируйте готовые записи."
        :"Подходящих фраз нет. Ниже показаны причины отказа и недостающие данные.";
    return out;
}
}

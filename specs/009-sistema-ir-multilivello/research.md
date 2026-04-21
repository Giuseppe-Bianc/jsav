# Phase 0 Research - Sistema IR Multi-Livello Verificabile

## Decision 1: Stack e toolchain canonici
- Decision: Usare C++23 con MSVC 2026 come baseline, CMake 4.2+ e Ninja.
- Rationale: Coerenza con costituzione (compatibilita VS 2026), build riproducibile, toolchain gia standard nel repository.
- Alternatives considered: Clang-only toolchain (scartato: non baseline per team), GCC-only (scartato: riduce allineamento con ambiente primario).

## Decision 2: Architettura monolite modulare
- Decision: Mantenere architettura monolite modulare con sottosistemi IR/analysis/passes/validation.
- Rationale: Dominio compiler ad alta coesione, semplifica transazioni pass, riduce costo operativo rispetto a sistemi distribuiti.
- Alternatives considered: Microservizi (scartato: overhead rete/osservabilita non giustificato), plugin runtime dinamici (scartato: complessita ABI).

## Decision 3: Dipendenze ammesse e pinning
- Decision: Usare solo fmtlib 12.1.0, spdlog 1.17.0, CLI11 2.6.1, Catch2 3.14.0 con version lock esistente.
- Rationale: Vincolo esplicito di progetto (nessuna nuova dipendenza), stabilita e auditabilita.
- Alternatives considered: Aggiunta librerie dataflow/graph esterne (scartato: violazione vincolo + costo integrazione).

## Decision 4: Error model unificato
- Decision: Tutti i fallimenti recuperabili passano da CompileError e da std::expected<T, std::vector<CompileError>>.
- Rationale: Supporta reporting batch deterministico per pass (FR-026), semplifica gestione errori cross-modulo.
- Alternatives considered: Eccezioni per validazione (scartato: difficile batching deterministico), codici errore numerici (scartato: contesto insufficiente).

## Decision 5: Canonical SSA/PHI
- Decision: Placement PHI canonico basato su reaching definitions (dataflow iterativo con sparse bitset), con pruning/minimizzazione eager su aggiornamenti CFG.
- Rationale: Aderenza diretta a FR-006/007/009 e riduzione PHI ridondanti rispetto a dominanza pura.
- Alternatives considered: Dominance frontier come criterio primario (scartato: over-approximation), SSA incrementale senza verifica RD finale (scartato: non canonico).

## Decision 6: Tracciabilita cross-level
- Decision: ID globali immutabili, deterministici da percorso canonico strutturale (modulo/funzione/blocco/indice/tipo entita) con relazioni di derivazione esplicite.
- Rationale: Audit robusto HIR->MIR->LIR e ordinamento stabile output/errori/report.
- Alternatives considered: UUID random (scartato: non deterministico), contatori runtime globali (scartato: dipendenza da ordine esecuzione).

## Decision 7: Data e stato transazionale
- Decision: Modello in-memory graph tipizzato con working copy transazionale per funzione/pass; commit atomico solo post-validazione.
- Rationale: Garantisce FR-022/SC-008 e impedisce stati intermedi invalidi persistenti.
- Alternatives considered: Mutazione in-place con undo log (scartato: maggiore superficie di bug), snapshot full-module per ogni pass (scartato: costo memoria eccessivo).

## Decision 8: Strategia strict no-reorder memoria
- Decision: In presenza di may-alias, riordino vietato salvo prova formale (certificato verificabile o dimostrazione interna riproducibile con log completo).
- Rationale: Preserva equivalenza osservabile su memoria (FR-028/030) e riduce regressioni semantiche.
- Alternatives considered: Heuristics di riordino permissive (scartato: rischio non determinismo/unsoundness).

## Decision 9: Test pyramid e copertura
- Decision: Usare tre livelli: constexpr compile-time, relaxed constexpr debug runtime, runtime completo; includere edge/corner case sistematici.
- Rationale: Bilancia verifica formale compile-time e comportamento dinamico su condizioni normali/anomale.
- Alternatives considered: Solo test runtime (scartato: perdita garanzie compile-time), solo property-based fuzzing (scartato: non sostituisce test deterministici).

## Decision 10: Quality gates CI/CD
- Decision: GitHub Actions con gate obbligatori: build zero warnings, test, clang-tidy/cppcheck, ASan/UBSan, lizard, gcovr >= 95%.
- Rationale: Enforcement automatico costituzione + criteri di successo SC.
- Alternatives considered: Validazione solo locale (scartato: drift tra ambienti), coverage non vincolante (scartato: rischio branch non testati).

## Decision 11: Configurazione ambienti
- Decision: Configurazioni iniettate via CMake Presets e variabili ambiente, nessun segreto in repository.
- Rationale: Parita ambiente dev/CI/release e tracciabilita configurazione.
- Alternatives considered: Config hardcoded (scartato: anti-pattern), .env versionati (scartato: rischio sicurezza).

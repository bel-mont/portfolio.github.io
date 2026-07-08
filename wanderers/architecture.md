# Architecture
Sample code can be found under the code samples folder.
- [WanderersTokenResolver.h](code_samples/WanderersTokenResolver.h)
- [WanderersBattleBoard.h](code_samples/WanderersBattleBoard.h)
- [WanderersActionSystemComponent.h](code_samples/WanderersActionSystemComponent.h)

```mermaid
flowchart TB
    subgraph Data["1 · Data / Config"]
        direction LR
        EffectAsset["Effect Asset\n(tags + attribute modifiers)"]
        TokenData["Token Data Asset\n(token definitions)"]
    end

    subgraph AbilitySystem["2 · Ability System (per Character)"]
        direction LR
        Action["Action\n(requirements · cooldown · effects)"]
        ASC["Action System Component\n(tags · attributes · actions)"]
    end

    subgraph Characters["3 · Characters"]
        direction LR
        Weapon["Weapon\n(grants / revokes actions)"]
        Character["Character Base\n(health · position · weapon)"]
    end

    subgraph Combat["4 · Combat"]
        direction LR
        TurnManager["Turn Manager\n(round / turn order)"]
        BattleBoard["Battle Board\n(central hub · drives combat)"]
        TokenResolver["Token Resolver\n(resolves hits · applies effects)"]
    end

    subgraph PlayerLayer["5 · Player"]
        Controller["Player Controller\n(input · targeting · pending action)"]
    end

    subgraph UI["6 · UI"]
        direction LR
        HUD["Battle HUD"]
        ActionPanel["Action Slot Panel"]
        ActionSlot["Action Slot\n(click to select)"]
        TokenWidget["Effects Widget\n(token display)"]
    end

%% ── Main downward flow ──────────────────────
    EffectAsset   --> Action
    TokenData     --> TokenResolver
    Action        --> ASC
    Weapon        --> ASC
    ASC           --> Character
    Character     --> BattleBoard
    TurnManager   --> BattleBoard
    TokenResolver --> BattleBoard
    BattleBoard   --> Controller
    Controller    --> HUD
    HUD           --> ActionPanel
    ActionPanel   --> ActionSlot

%% ── Action-selection feedback (numbered for readability) ──
    ActionSlot  -. "① click" .->              ActionPanel
    ActionPanel -. "② selected" .->           Controller
    Controller  -. "③ ResolveAction" .->      BattleBoard
    BattleBoard -. "④ ResolveHit" .->         TokenResolver

%% ── State change → token UI ─────────────────
    ASC         -. "tag changed" .->           Character
    Character   -. "UpdateTokenWidgets" .->    TokenWidget

%% ── Highlight key nodes ─────────────────────
    style ASC         fill:#1a3a5c,color:#fff,stroke:#4a9eff
    style BattleBoard fill:#1a3a5c,color:#fff,stroke:#4a9eff
```
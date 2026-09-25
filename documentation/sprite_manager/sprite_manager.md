``` mermaid
    flowchart TD
        SR["Sprite Registry"] --> SD["Shared Sprite Definition"]
        EM["Enemy Manager"] --> E["Enemy"]
        E --> U["Unit"]
        U --> AI["Animation Instance"]
        AI --> SD
        EM --> R["Sprite Renderer"]
        R --> SR
```
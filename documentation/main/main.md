``` mermaid
    flowchart TD
        A([Program Start: main]) --> B[/Print build mode/]
        B --> C[Initialize SDL logger]
        C --> D{SDL_Init succeeds?}

        D -- No --> D1[/Log SDL initialization failure/]
        D1 --> D2([EXIT_FAILURE])

        D -- Yes --> E[Create SDL window]
        E --> F{Window created?}
        F -- No --> F1[/Log window creation failure/]
        F1 --> F2[SDL_Quit]
        F2 --> F3([EXIT_FAILURE])

        F -- Yes --> G[Create OpenGL context]
        G --> H{OpenGL context created?}
        H -- No --> H1[/Log OpenGL context failure/]
        H1 --> H2[SDL_Quit]
        H2 --> H3([EXIT_FAILURE])

        H -- Yes --> I[Make context current and enable VSync]
        I --> J[Initialize GLUT and GLEW]
        J --> K{GLEW initialized?}

        K -- No --> K1[Destroy context and window]
        K1 --> K2[SDL_Quit]
        K2 --> K3([EXIT_FAILURE])

        K -- Yes --> L[Initialize sound engine]
        L --> M{Sound engine initialized?}
        M -- No --> M1[/Log sound initialization failure/]
        M1 --> M2[SDL_Quit]
        M2 --> M3([EXIT_FAILURE])

        M -- Yes --> N[Create and initialize game scene]
        N --> O[Resize scene and configure OpenGL viewport]
        O --> P[Create and initialize menu manager]
        P --> Q[Load home menu page]
        Q --> R[Initialize timing variables]
        R --> S[Set running = true]

        S --> T{running?}
        T -- No --> Z[Log game exit]
        T -- Yes --> U[Reset mouse wheel input]

        U --> V{SDL event available?}
        V -- Yes --> W{Event type}

        W -- Quit --> W1[Set running = false]
        W -- Window resized --> W2[Resize window and scene]
        W -- Mouse motion --> W3[Update mouse position]
        W -- Mouse button --> W4[Update button state]
        W -- Mouse wheel --> W5[Update wheel input and notify scene]
        W -- Key down --> W6[Update key state and process shortcuts]
        W -- Key up --> W7[Clear key state]
        W -- Other --> W8[Ignore event]

        W1 --> V
        W2 --> V
        W3 --> V
        W4 --> V
        W5 --> V
        W6 --> V
        W7 --> V
        W8 --> V

        V -- No --> X[Calculate delta time]
        X --> Y{Delta time >= update delay?}

        Y -- Yes --> Y1[Update game or menu]
        Y1 --> Y2[Update audio]
        Y2 --> Y3[Store current update time]
        Y3 --> AA[Update scene background]
        Y -- No --> AA

        AA --> AB[Draw game scene or menu]
        AB --> AC[Swap OpenGL window buffers]
        AC --> T

        Z --> ZA[Release sound engine]
        ZA --> ZB[SDL_Quit]
        ZB --> ZC([EXIT_SUCCESS])
```

https://www.geeksforgeeks.org/dsa/an-introduction-to-flowcharts/


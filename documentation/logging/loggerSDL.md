``` mermaid
    flowchart TD
        T.0["GG_LOG_TRACE(category, message)"]
        D.0["GG_LOG_DEBUG(category, message)"]
        I.0["GG_LOG_INFO(category, message)"]
        W.0["GG_LOG_WARN(category, message)"]
        E.0["GG_LOG_ERROR(category, message)"]
        C.0["GG_LOG_CRITICAL(category, message)"]

        T.0 --> T.1["SDL_LogTrace(category, message)"]
        D.0 --> D.1["SDL_LogDebug(category, message)"]
        I.0 --> I.1["SDL_LogInfo(category, message)"]
        W.0 --> W.1["SDL_LogWarn(category, message)"]
        E.0 --> E.1["SDL_LogError(category, message)"]
        C.0 --> C.1["SDL_LogCritical(category, message)"]

        GLO["gameLogOutput(userdata, category, priority, message)"]

        T.1 --> GLO
        D.1 --> GLO
        I.1 --> GLO
        W.1 --> GLO
        E.1 --> GLO
        C.1 --> GLO

        GLO --> GLO.A["getCategoryName(category)"]
        GLO --> GLO.B["getPriorityName(priority)"]

        GLO --> GLO.C{"Is game in debug mode?"}

        GLO.C --> |Yes| GLO.C.T["Log to console"]
        GLO.C --> |No| GLO.D["Log to file"]

        GLO.C.T --> GLO.D
```


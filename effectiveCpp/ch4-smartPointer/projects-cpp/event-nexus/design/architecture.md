# Architecture

## Component Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                        User Application                         │
│                                                                 │
│  ┌──────────┐  publishSync<T>()   ┌────────────────────────┐   │
│  │ Publisher │ ──────────────────▶ │       EventBus         │   │
│  └──────────┘  publishAsync<T>()  │   (Pimpl: unique_ptr)  │   │
│                                    └───────────┬────────────┘   │
│  ┌──────────┐  subscribe<T>()          ┌───────┴────────┐      │
│  │  Owner   │ ◀── shared_ptr ──────▶   │  Composition   │      │
│  └──────────┘     (subscription)       └───┬────────┬───┘      │
│       │                                    │        │           │
│       │ .reset()                           ▼        ▼           │
│       ▼                         ┌──────────────┐ ┌──────────┐  │
│  (auto-expire)                  │  Dispatcher  │ │ThreadPool│  │
│                                 │ (Pimpl)      │ │ (Pimpl)  │  │
│                                 └──────┬───────┘ └──────────┘  │
│                                        │                        │
│                            weak_ptr<ISubscriber>                │
│                                  per event type                 │
│                                        │                        │
│                              ┌─────────┼──────────┐            │
│                              ▼         ▼          ▼            │
│                          ┌───────┐ ┌───────┐ ┌───────┐        │
│                          │Sub #1 │ │Sub #2 │ │Sub #3 │        │
│                          │(alive)│ │(alive)│ │(dead) │        │
│                          └───────┘ └───────┘ └───────┘        │
└─────────────────────────────────────────────────────────────────┘
```

## Data Flow — Synchronous Publish

```
Publisher                 EventBus              Dispatcher          Subscribers
    │                        │                      │                    │
    │  publishSync<T>(args)  │                      │                    │
    │───────────────────────▶│                      │                    │
    │                        │  makeEvent<T>(args)  │                    │
    │                        │  → unique_ptr        │                    │
    │                        │  dispatch(*event)    │                    │
    │                        │─────────────────────▶│                    │
    │                        │                      │ lock mutex         │
    │                        │                      │ copy live subs     │
    │                        │                      │ unlock mutex       │
    │                        │                      │ onEvent(event)     │
    │                        │                      │───────────────────▶│
    │                        │                      │   (for each sub)   │
    │                        │                      │◀───────────────────│
    │  return                │                      │                    │
    │◀───────────────────────│                      │                    │
```

## Data Flow — Asynchronous Publish

```
Publisher                 EventBus              ThreadPool          Dispatcher
    │                        │                      │                    │
    │  publishAsync<T>(args) │                      │                    │
    │───────────────────────▶│                      │                    │
    │                        │ unique_ptr → shared_ptr                   │
    │                        │ submit(lambda)       │                    │
    │                        │─────────────────────▶│                    │
    │  return (immediate)    │                      │                    │
    │◀───────────────────────│                      │                    │
    │                        │                      │  worker picks up   │
    │                        │                      │  dispatch(*event)  │
    │                        │                      │───────────────────▶│
    │                        │                      │                    │─▶ subscribers
```

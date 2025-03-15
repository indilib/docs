---
title: Driver Loop
nav_order: 3
parent: Advanced
---

## Driver Loop

You'll probably want to do more than just respond to user input in your driver.
If you need to periodically check things, the main driver loop is where to do it.

At it's most basic, you really just need to override `TimerHit`.

```cpp
void MyCustomDriver::TimerHit()
{
    if (!isConnected())
        return;

    LOG_INFO("timer hit");

    // If you don't call SetTimer, we'll never get called again, until we disconnect
    // and reconnect.
    SetTimer(POLLMS);
}
```

After `Handshake` is called when connecting, `DefaultDevice` will call `TimerHit` after `POLLMS` elapses. We have to make sure to set the timer again at the end, otherwise it'll never get called
again (until we reconnect that is).

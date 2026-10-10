# Boot-stage breadcrumbs

noOS maintains a small in-memory boot status structure with magic `NOOS` (`0x4E4F4F53`).

The current stages are:

| Stage | Meaning |
| --- | --- |
| 1 | reset |
| 2 | entered C |
| 3 | boot info parsed |
| 4 | PMM initialized |
| 5 | heap initialized |
| 6 | self-tests completed |
| 7 | kernel ready/idle |
| 255 | panic |

The `detail` field is stage-specific. Current uses include the detected boot protocol, free-page count, self-test failure count, and early-console length.

These breadcrumbs are useful when the phone gives no visible output. A debugger, memory dump, or future ramoops-style export can inspect the structure and identify how far boot progressed.

The status block is diagnostic state only; it is not a persistent crash log across power cycles.

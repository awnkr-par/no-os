# Kernel circular log

The original early console retained only the first 4 KiB of boot text.

noOS now mirrors every console character into an 8 KiB circular kernel log.

Properties:

- fixed-size, allocation-free storage;
- oldest data is overwritten after the ring fills;
- wrap and overwrite counters are tracked;
- snapshots return the newest retained bytes in chronological order;
- panic/diagnostic capture uses the circular log rather than the old first-4-KiB buffer.

The legacy early console buffer is still kept for simple bring-up consumers, but the circular log is the canonical long-running kernel log.

The diagnostic record captures:

- total bytes written;
- bytes overwritten;
- wrap count;
- retained length;
- the newest diagnostic log tail.

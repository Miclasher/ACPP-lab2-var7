```
================================================================================
Starting experiment for data size: 100000000 elements
================================================================================

--- Standard Library Algorithms ---
1) No policy:            992.2276 ms
2) execution::seq:       987.8999 ms
3) execution::par:       103.6477 ms (Speedup vs seq: 9.5313x)
4) execution::par_unseq:   111.0710 ms (Speedup vs seq: 8.8943x)

--- Custom Parallel Algorithm ---
Number of hardware threads on this system: 12

+-------+----------------+----------+
|   K   |  Time (ms)     | Speedup  |
+-------+----------------+----------+
|     1 |       993.3720 |     1.00x |
|     2 |       508.8923 |     1.94x |
|     3 |       340.6385 |     2.90x |
|     4 |       264.4103 |     3.74x |
|     5 |       215.2427 |     4.59x |
|     6 |       186.9178 |     5.29x |
|     7 |       163.7421 |     6.03x |
|     8 |       144.8301 |     6.82x |
|     9 |       127.4282 |     7.75x |
|    10 |       115.9360 |     8.52x |
|    11 |       108.3705 |     9.12x |
|    12 |       105.7809 |     9.34x |
|    13 |       121.5136 |     8.13x |
|    14 |       118.1051 |     8.36x |
|    15 |       115.5091 |     8.55x |
|    16 |       116.4897 |     8.48x |
|    17 |       129.4137 |     7.63x |
|    18 |       127.3517 |     7.76x |
|    19 |       119.4565 |     8.27x |
|    20 |       115.9268 |     8.52x |
|    21 |       112.5025 |     8.78x |
|    22 |       109.8016 |     9.00x |
|    23 |       106.0472 |     9.32x |
|    24 |       111.4755 |     8.86x |
+-------+----------------+----------+

--- Findings for this data size ---
Best performance for custom algorithm was achieved at K = 12 threads.
This corresponds to 1.00 times the number of available hardware threads (12).
The best speedup achieved was 9.34x compared to the sequential version.

================================================================================
All experiments completed.
Final result sink (to prevent optimization that removes time measurement): -40059107.96
================================================================================
```

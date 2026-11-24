#!/usr/bin/env python3
"""Check an OrCS SYNC_DEBUG log (build with SYNC_DEBUG 1 in defines.hpp).

Usage: python3 scripts/check_sync.py <orcs output> <number of cores>

Checks, from the "SYNC <cycle> <core> <event> ..." lines:
  1. Barrier order: every core still running arrives at barrier g before
     any core passes it (cores that retired earlier count as arrived).
  2. Lock exclusion: no LOCK_ACQUIRE while another core holds the lock,
     and no LOCK_RELEASE by a core that does not hold it.
  3. Store -> load order: the last store committed by any core before
     barrier g happens before the first load any core sends after it.
Prints one line per barrier and RESULT PASS/FAIL.
"""
import sys, collections

ev=[l.split() for l in open(sys.argv[1]) if l.startswith('SYNC ')]
ev=[(int(c),int(k),e,r) for _,c,k,e,*r in ev]
ncores=int(sys.argv[2]); ok=True
arrive=collections.defaultdict(dict); pas=collections.defaultdict(dict); retire={}
owner=None; last_store=collections.defaultdict(dict); first_load=collections.defaultdict(dict); cur_gen={}
for c,k,e,r in ev:
    if e=='ARRIVE': g=int(r[0]); arrive[g][k]=c; last_store[g][k]=int(r[1].split('=')[1])
    elif e=='PASS': g=int(r[0]); pas[g][k]=c; cur_gen[k]=g
    elif e=='FIRST_LOAD' and k in cur_gen: first_load[cur_gen.pop(k)][k]=c
    elif e=='RETIRE': retire[k]=c
    elif e=='LOCK_ACQUIRE':
        if owner is not None: print(f"FAIL cycle {c}: core {k} acquired while core {owner} holds the lock"); ok=False
        owner=k
    elif e=='LOCK_RELEASE':
        if owner!=k: print(f"FAIL cycle {c}: core {k} released a lock it does not hold"); ok=False
        owner=None
for g in sorted(pas):
    # every core not retired before the barrier opened must have arrived before anyone passed
    first_pass=min(pas[g].values())
    expected=[k for k in range(ncores) if not (k in retire and retire[k]<=first_pass)]
    missing=[k for k in expected if k not in arrive[g]]
    late=[k for k in arrive[g] if arrive[g][k]>first_pass]
    if missing or late: print(f"FAIL barrier {g}: missing {missing} late {late}"); ok=False
    st=max(last_store[g].values()); ld=min(first_load[g].values()) if first_load[g] else None
    if ld is not None and ld<=st: print(f"FAIL barrier {g}: load at {ld} before store commit {st}"); ok=False
    print(f"barrier {g}: arrived {sorted(arrive[g])} by {max(arrive[g].values())}, first pass {first_pass}, "
          f"last store commit {st}, first load after {ld}")
print(f"locks: {sum(e=='LOCK_ACQUIRE' for _,_,e,_ in ev)} acquisitions, mutual exclusion "+("OK" if ok else "?"))
print("RESULT", "PASS" if ok else "FAIL")

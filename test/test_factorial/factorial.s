.global factorial

.section my_factorial
factorial:
  ld [%sp + 4], %r2
  bne %r2, %r1, fact
  ret

fact:
  sub %r1, %r2
  push %r2
  call factorial
  pop %r3
  mul %r3, %r2
  ret

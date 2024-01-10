
.extern factorial

.section start
main:
  ld $0xFFFFFEFE, %sp
  push %r3
  ld $4, %r1
  push %r1
  ld $1, %r1
  call factorial
  pop %r1
  mul %r1, %r2
  pop %r3
  halt

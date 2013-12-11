; RUN: llc  -march=cpu0 -mcpu=cpu032I  -relocation-model=pic -O3 %s -o - | FileCheck %s -check-prefix=cpu032I
; RUN: llc  -march=cpu0 -mcpu=cpu032II  -relocation-model=pic -O3 %s -o - | FileCheck %s -check-prefix=cpu032II

@i = global i32 0, align 4
@r1 = common global i32 0, align 4

define void @test() nounwind {
entry:
  %0 = load i32* @i, align 4
  %cmp = icmp eq i32 %0, 0
  %conv = zext i1 %cmp to i32
  store i32 %conv, i32* @r1, align 4
; cpu032I:  cmp	$sw, ${{[0-9]+}}, ${{[0-9]+}}
; cpu032I:  mfsw	$[[T0:[0-9]+]]
; cpu032I:  andi	$[[T1:[0-9]+]], $[[T0]], 2
; cpu032I:  shr	${{[0-9]+}}, $[[T1]], 1
; cpu032II:  xor	$[[T0:[0-9]+]], ${{[0-9]+}}, ${{[0-9]+}}
; cpu032II:  sltiu	${{[0-9]+}}, $[[T0]], 1

  ret void
}

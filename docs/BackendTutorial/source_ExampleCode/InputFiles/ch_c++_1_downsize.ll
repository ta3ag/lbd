; ModuleID = 'ch_c++_1.cpp'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.CPolygon = type { i32 (...)**, i32, i32 }

; Function Attrs: uwtable
define linkonce_odr void @_ZN8CPolygon9printareaEv(%class.CPolygon* %this) #0 align 2 {
  %1 = alloca %class.CPolygon*, align 8
  store %class.CPolygon* %this, %class.CPolygon** %1, align 8
  %2 = load %class.CPolygon** %1
  %3 = bitcast %class.CPolygon* %2 to i32 (%class.CPolygon*)***
  %4 = load i32 (%class.CPolygon*)*** %3
  %5 = getelementptr inbounds i32 (%class.CPolygon*)** %4, i64 0
  %6 = load i32 (%class.CPolygon*)** %5
  %7 = call i32 %6(%class.CPolygon* %2)
  ret void
}

!llvm.ident = !{!0}

!0 = metadata !{metadata !"clang version 3.4 (tags/RELEASE_34/final)"}

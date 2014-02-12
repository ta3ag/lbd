; ModuleID = 'ch_c++_1.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.9.0"

%"class.std::__1::basic_ostream" = type { i32 (...)**, %"class.std::__1::basic_ios" }
%"class.std::__1::basic_ios" = type { %"class.std::__1::ios_base", %"class.std::__1::basic_ostream"*, i32 }
%"class.std::__1::ios_base" = type { i32 (...)**, i32, i64, i64, i32, i32, i8*, i8*, void (i32, %"class.std::__1::ios_base"*, i32)**, i32*, i64, i64, i64*, i64, i64, i8**, i64, i64 }
%"class.std::__1::locale::id" = type { %"struct.std::__1::once_flag", i32 }
%"struct.std::__1::once_flag" = type { i64 }
%class.CPolygon = type { i32 (...)**, i32, i32 }
%class.CRectangle = type { %class.CPolygon }
%class.CTriangle = type { %class.CPolygon }
%"class.std::__1::ctype" = type { %"class.std::__1::locale::facet", i32*, i8 }
%"class.std::__1::locale::facet" = type { %"class.std::__1::__shared_count" }
%"class.std::__1::__shared_count" = type { i32 (...)**, i64 }
%"class.std::__1::locale" = type { %"class.std::__1::locale::__imp"* }
%"class.std::__1::locale::__imp" = type opaque

@_ZNSt3__14coutE = external global %"class.std::__1::basic_ostream"
@_ZNSt3__15ctypeIcE2idE = external global %"class.std::__1::locale::id"
@_ZTV9CTriangle = linkonce_odr unnamed_addr constant [3 x i8*] [i8* null, i8* bitcast ({ i8*, i8*, i8* }* @_ZTI9CTriangle to i8*), i8* bitcast (i32 (%class.CTriangle*)* @_ZN9CTriangle4areaEv to i8*)]
@_ZTVN10__cxxabiv120__si_class_type_infoE = external global i8*
@_ZTS9CTriangle = linkonce_odr constant [11 x i8] c"9CTriangle\00"
@_ZTVN10__cxxabiv117__class_type_infoE = external global i8*
@_ZTS8CPolygon = linkonce_odr constant [10 x i8] c"8CPolygon\00"
@_ZTI8CPolygon = linkonce_odr unnamed_addr constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8** @_ZTVN10__cxxabiv117__class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([10 x i8]* @_ZTS8CPolygon, i32 0, i32 0) }
@_ZTI9CTriangle = linkonce_odr unnamed_addr constant { i8*, i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8** @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([11 x i8]* @_ZTS9CTriangle, i32 0, i32 0), i8* bitcast ({ i8*, i8* }* @_ZTI8CPolygon to i8*) }
@_ZTV8CPolygon = linkonce_odr unnamed_addr constant [3 x i8*] [i8* null, i8* bitcast ({ i8*, i8* }* @_ZTI8CPolygon to i8*), i8* bitcast (void ()* @__cxa_pure_virtual to i8*)]
@_ZTV10CRectangle = linkonce_odr unnamed_addr constant [3 x i8*] [i8* null, i8* bitcast ({ i8*, i8*, i8* }* @_ZTI10CRectangle to i8*), i8* bitcast (i32 (%class.CRectangle*)* @_ZN10CRectangle4areaEv to i8*)]
@_ZTS10CRectangle = linkonce_odr constant [13 x i8] c"10CRectangle\00"
@_ZTI10CRectangle = linkonce_odr unnamed_addr constant { i8*, i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8** @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([13 x i8]* @_ZTS10CRectangle, i32 0, i32 0), i8* bitcast ({ i8*, i8* }* @_ZTI8CPolygon to i8*) }

; Function Attrs: ssp uwtable
define i32 @main() #0 {
  %1 = alloca i32, align 4
  %ppoly1 = alloca %class.CPolygon*, align 8
  %ppoly2 = alloca %class.CPolygon*, align 8
  store i32 0, i32* %1
  %2 = call noalias i8* @_Znwm(i64 16)
  %3 = bitcast i8* %2 to %class.CRectangle*
  call void @_ZN10CRectangleC1Ev(%class.CRectangle* %3) #3
  %4 = bitcast %class.CRectangle* %3 to %class.CPolygon*
  store %class.CPolygon* %4, %class.CPolygon** %ppoly1, align 8
  %5 = call noalias i8* @_Znwm(i64 16)
  %6 = bitcast i8* %5 to %class.CTriangle*
  call void @_ZN9CTriangleC1Ev(%class.CTriangle* %6) #3
  %7 = bitcast %class.CTriangle* %6 to %class.CPolygon*
  store %class.CPolygon* %7, %class.CPolygon** %ppoly2, align 8
  %8 = load %class.CPolygon** %ppoly1, align 8
  call void @_ZN8CPolygon10set_valuesEii(%class.CPolygon* %8, i32 4, i32 5)
  %9 = load %class.CPolygon** %ppoly2, align 8
  call void @_ZN8CPolygon10set_valuesEii(%class.CPolygon* %9, i32 4, i32 5)
  %10 = load %class.CPolygon** %ppoly1, align 8
  call void @_ZN8CPolygon9printareaEv(%class.CPolygon* %10)
  %11 = load %class.CPolygon** %ppoly2, align 8
  call void @_ZN8CPolygon9printareaEv(%class.CPolygon* %11)
  %12 = load %class.CPolygon** %ppoly1, align 8
  %13 = icmp eq %class.CPolygon* %12, null
  br i1 %13, label %16, label %14

; <label>:14                                      ; preds = %0
  %15 = bitcast %class.CPolygon* %12 to i8*
  call void @_ZdlPv(i8* %15) #3
  br label %16

; <label>:16                                      ; preds = %14, %0
  %17 = load %class.CPolygon** %ppoly2, align 8
  %18 = icmp eq %class.CPolygon* %17, null
  br i1 %18, label %21, label %19

; <label>:19                                      ; preds = %16
  %20 = bitcast %class.CPolygon* %17 to i8*
  call void @_ZdlPv(i8* %20) #3
  br label %21

; <label>:21                                      ; preds = %19, %16
  ret i32 0
}

declare noalias i8* @_Znwm(i64)

; Function Attrs: inlinehint nounwind ssp uwtable
define linkonce_odr void @_ZN10CRectangleC1Ev(%class.CRectangle* %this) unnamed_addr #1 align 2 {
  %1 = alloca %class.CRectangle*, align 8
  store %class.CRectangle* %this, %class.CRectangle** %1, align 8
  %2 = load %class.CRectangle** %1
  call void @_ZN10CRectangleC2Ev(%class.CRectangle* %2) #3
  ret void
}

; Function Attrs: inlinehint nounwind ssp uwtable
define linkonce_odr void @_ZN9CTriangleC1Ev(%class.CTriangle* %this) unnamed_addr #1 align 2 {
  %1 = alloca %class.CTriangle*, align 8
  store %class.CTriangle* %this, %class.CTriangle** %1, align 8
  %2 = load %class.CTriangle** %1
  call void @_ZN9CTriangleC2Ev(%class.CTriangle* %2) #3
  ret void
}

; Function Attrs: nounwind ssp uwtable
define linkonce_odr void @_ZN8CPolygon10set_valuesEii(%class.CPolygon* %this, i32 %a, i32 %b) #2 align 2 {
  %1 = alloca %class.CPolygon*, align 8
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  store %class.CPolygon* %this, %class.CPolygon** %1, align 8
  store i32 %a, i32* %2, align 4
  store i32 %b, i32* %3, align 4
  %4 = load %class.CPolygon** %1
  %5 = load i32* %2, align 4
  %6 = getelementptr inbounds %class.CPolygon* %4, i32 0, i32 1
  store i32 %5, i32* %6, align 4
  %7 = load i32* %3, align 4
  %8 = getelementptr inbounds %class.CPolygon* %4, i32 0, i32 2
  store i32 %7, i32* %8, align 4
  ret void
}

; Function Attrs: ssp uwtable
define linkonce_odr void @_ZN8CPolygon9printareaEv(%class.CPolygon* %this) #0 align 2 {
  %1 = alloca %"class.std::__1::basic_ostream"*, align 8
  %2 = alloca %"class.std::__1::basic_ostream"* (%"class.std::__1::basic_ostream"*)*, align 8
  %3 = alloca %class.CPolygon*, align 8
  store %class.CPolygon* %this, %class.CPolygon** %3, align 8
  %4 = load %class.CPolygon** %3
  %5 = bitcast %class.CPolygon* %4 to i32 (%class.CPolygon*)***
  %6 = load i32 (%class.CPolygon*)*** %5
  %7 = getelementptr inbounds i32 (%class.CPolygon*)** %6, i64 0
  %8 = load i32 (%class.CPolygon*)** %7
  %9 = call i32 %8(%class.CPolygon* %4)
  %10 = call %"class.std::__1::basic_ostream"* @_ZNSt3__113basic_ostreamIcNS_11char_traitsIcEEElsEi(%"class.std::__1::basic_ostream"* @_ZNSt3__14coutE, i32 %9)
  store %"class.std::__1::basic_ostream"* %10, %"class.std::__1::basic_ostream"** %1, align 8
  store %"class.std::__1::basic_ostream"* (%"class.std::__1::basic_ostream"*)* @_ZNSt3__14endlIcNS_11char_traitsIcEEEERNS_13basic_ostreamIT_T0_EES7_, %"class.std::__1::basic_ostream"* (%"class.std::__1::basic_ostream"*)** %2, align 8
  %11 = load %"class.std::__1::basic_ostream"** %1
  %12 = load %"class.std::__1::basic_ostream"* (%"class.std::__1::basic_ostream"*)** %2, align 8
  %13 = call %"class.std::__1::basic_ostream"* %12(%"class.std::__1::basic_ostream"* %11)
  ret void
}

; Function Attrs: nounwind
declare void @_ZdlPv(i8*) #3

declare %"class.std::__1::basic_ostream"* @_ZNSt3__113basic_ostreamIcNS_11char_traitsIcEEElsEi(%"class.std::__1::basic_ostream"*, i32)

; Function Attrs: alwaysinline inlinehint ssp uwtable
define linkonce_odr hidden %"class.std::__1::basic_ostream"* @_ZNSt3__14endlIcNS_11char_traitsIcEEEERNS_13basic_ostreamIT_T0_EES7_(%"class.std::__1::basic_ostream"* %__os) #4 {
  %1 = alloca %"class.std::__1::ctype"*, align 8
  %2 = alloca i8, align 1
  %3 = alloca %"class.std::__1::locale"*, align 8
  %4 = alloca %"class.std::__1::basic_ios"*, align 8
  %5 = alloca i8, align 1
  %6 = alloca %"class.std::__1::locale", align 8
  %7 = alloca i8*
  %8 = alloca i32
  %9 = alloca %"class.std::__1::basic_ostream"*, align 8
  store %"class.std::__1::basic_ostream"* %__os, %"class.std::__1::basic_ostream"** %9, align 8
  %10 = load %"class.std::__1::basic_ostream"** %9, align 8
  %11 = load %"class.std::__1::basic_ostream"** %9, align 8
  %12 = bitcast %"class.std::__1::basic_ostream"* %11 to i8**
  %13 = load i8** %12
  %14 = getelementptr i8* %13, i64 -24
  %15 = bitcast i8* %14 to i64*
  %16 = load i64* %15
  %17 = bitcast %"class.std::__1::basic_ostream"* %11 to i8*
  %18 = getelementptr inbounds i8* %17, i64 %16
  %19 = bitcast i8* %18 to %"class.std::__1::basic_ios"*
  store %"class.std::__1::basic_ios"* %19, %"class.std::__1::basic_ios"** %4, align 8
  store i8 10, i8* %5, align 1
  %20 = load %"class.std::__1::basic_ios"** %4
  %21 = bitcast %"class.std::__1::basic_ios"* %20 to %"class.std::__1::ios_base"*
  call void @_ZNKSt3__18ios_base6getlocEv(%"class.std::__1::locale"* sret %6, %"class.std::__1::ios_base"* %21)
  store %"class.std::__1::locale"* %6, %"class.std::__1::locale"** %3, align 8
  %22 = load %"class.std::__1::locale"** %3, align 8
  %23 = invoke %"class.std::__1::locale::facet"* @_ZNKSt3__16locale9use_facetERNS0_2idE(%"class.std::__1::locale"* %22, %"class.std::__1::locale::id"* @_ZNSt3__15ctypeIcE2idE)
          to label %_ZNSt3__19use_facetINS_5ctypeIcEEEERKT_RKNS_6localeE.exit.i unwind label %33

_ZNSt3__19use_facetINS_5ctypeIcEEEERKT_RKNS_6localeE.exit.i: ; preds = %0
  %24 = bitcast %"class.std::__1::locale::facet"* %23 to %"class.std::__1::ctype"*
  %25 = load i8* %5, align 1
  store %"class.std::__1::ctype"* %24, %"class.std::__1::ctype"** %1, align 8
  store i8 %25, i8* %2, align 1
  %26 = load %"class.std::__1::ctype"** %1
  %27 = bitcast %"class.std::__1::ctype"* %26 to i8 (%"class.std::__1::ctype"*, i8)***
  %28 = load i8 (%"class.std::__1::ctype"*, i8)*** %27
  %29 = getelementptr inbounds i8 (%"class.std::__1::ctype"*, i8)** %28, i64 7
  %30 = load i8 (%"class.std::__1::ctype"*, i8)** %29
  %31 = load i8* %2, align 1
  %32 = invoke signext i8 %30(%"class.std::__1::ctype"* %26, i8 signext %31)
          to label %_ZNKSt3__19basic_iosIcNS_11char_traitsIcEEE5widenEc.exit unwind label %33

; <label>:33                                      ; preds = %_ZNSt3__19use_facetINS_5ctypeIcEEEERKT_RKNS_6localeE.exit.i, %0
  %34 = landingpad { i8*, i32 } personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*)
          cleanup
;  %35 = extractvalue { i8*, i32 } %34, 0
  %35 = load i8** %12
  store i8* %35, i8** %7
;  %36 = extractvalue { i8*, i32 } %34, 1
  %36 = load i8** %12
  store i8* %35, i8** %7
  invoke void @_ZNSt3__16localeD1Ev(%"class.std::__1::locale"* %6)
          to label %37 unwind label %42

; <label>:37                                      ; preds = %33
  %38 = load i8** %7
  %39 = load i32* %8
  %40 = insertvalue { i8*, i32 } undef, i8* %38, 0
  %41 = insertvalue { i8*, i32 } %40, i32 %39, 1
  resume { i8*, i32 } %41

; <label>:42                                      ; preds = %33
  %43 = landingpad { i8*, i32 } personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*)
          catch i8* null
;  %43 = load i8* %7, align 1
  %44 = extractvalue { i8*, i32 } %43, 0
  call void @__clang_call_terminate(i8* %44) #6
  unreachable

_ZNKSt3__19basic_iosIcNS_11char_traitsIcEEE5widenEc.exit: ; preds = %_ZNSt3__19use_facetINS_5ctypeIcEEEERKT_RKNS_6localeE.exit.i
  call void @_ZNSt3__16localeD1Ev(%"class.std::__1::locale"* %6)
  %45 = call %"class.std::__1::basic_ostream"* @_ZNSt3__113basic_ostreamIcNS_11char_traitsIcEEE3putEc(%"class.std::__1::basic_ostream"* %10, i8 signext %32)
  %46 = load %"class.std::__1::basic_ostream"** %9, align 8
  %47 = call %"class.std::__1::basic_ostream"* @_ZNSt3__113basic_ostreamIcNS_11char_traitsIcEEE5flushEv(%"class.std::__1::basic_ostream"* %46)
  %48 = load %"class.std::__1::basic_ostream"** %9, align 8
  ret %"class.std::__1::basic_ostream"* %48
}

declare %"class.std::__1::basic_ostream"* @_ZNSt3__113basic_ostreamIcNS_11char_traitsIcEEE3putEc(%"class.std::__1::basic_ostream"*, i8 signext)

declare %"class.std::__1::basic_ostream"* @_ZNSt3__113basic_ostreamIcNS_11char_traitsIcEEE5flushEv(%"class.std::__1::basic_ostream"*)

declare void @_ZNKSt3__18ios_base6getlocEv(%"class.std::__1::locale"* sret, %"class.std::__1::ios_base"*)

declare i32 @__gxx_personality_v0(...)

declare void @_ZNSt3__16localeD1Ev(%"class.std::__1::locale"*)

; Function Attrs: noinline noreturn nounwind
define linkonce_odr hidden void @__clang_call_terminate(i8*) #5 {
  %2 = call i8* @__cxa_begin_catch(i8* %0) #3
  call void @_ZSt9terminatev() #6
  unreachable
}

declare i8* @__cxa_begin_catch(i8*)

declare void @_ZSt9terminatev()

declare %"class.std::__1::locale::facet"* @_ZNKSt3__16locale9use_facetERNS0_2idE(%"class.std::__1::locale"*, %"class.std::__1::locale::id"*)

; Function Attrs: inlinehint nounwind ssp uwtable
define linkonce_odr void @_ZN9CTriangleC2Ev(%class.CTriangle* %this) unnamed_addr #1 align 2 {
  %1 = alloca %class.CTriangle*, align 8
  store %class.CTriangle* %this, %class.CTriangle** %1, align 8
  %2 = load %class.CTriangle** %1
  %3 = bitcast %class.CTriangle* %2 to %class.CPolygon*
  call void @_ZN8CPolygonC2Ev(%class.CPolygon* %3) #3
  %4 = bitcast %class.CTriangle* %2 to i8***
  store i8** getelementptr inbounds ([3 x i8*]* @_ZTV9CTriangle, i64 0, i64 2), i8*** %4
  ret void
}

; Function Attrs: inlinehint nounwind ssp uwtable
define linkonce_odr void @_ZN8CPolygonC2Ev(%class.CPolygon* %this) unnamed_addr #1 align 2 {
  %1 = alloca %class.CPolygon*, align 8
  store %class.CPolygon* %this, %class.CPolygon** %1, align 8
  %2 = load %class.CPolygon** %1
  %3 = bitcast %class.CPolygon* %2 to i8***
  store i8** getelementptr inbounds ([3 x i8*]* @_ZTV8CPolygon, i64 0, i64 2), i8*** %3
  ret void
}

; Function Attrs: nounwind ssp uwtable
define linkonce_odr i32 @_ZN9CTriangle4areaEv(%class.CTriangle* %this) unnamed_addr #2 align 2 {
  %1 = alloca %class.CTriangle*, align 8
  store %class.CTriangle* %this, %class.CTriangle** %1, align 8
  %2 = load %class.CTriangle** %1
  %3 = bitcast %class.CTriangle* %2 to %class.CPolygon*
  %4 = getelementptr inbounds %class.CPolygon* %3, i32 0, i32 1
  %5 = load i32* %4, align 4
  %6 = bitcast %class.CTriangle* %2 to %class.CPolygon*
  %7 = getelementptr inbounds %class.CPolygon* %6, i32 0, i32 2
  %8 = load i32* %7, align 4
  %9 = mul nsw i32 %5, %8
  %10 = sdiv i32 %9, 2
  ret i32 %10
}

declare void @__cxa_pure_virtual()

; Function Attrs: inlinehint nounwind ssp uwtable
define linkonce_odr void @_ZN10CRectangleC2Ev(%class.CRectangle* %this) unnamed_addr #1 align 2 {
  %1 = alloca %class.CRectangle*, align 8
  store %class.CRectangle* %this, %class.CRectangle** %1, align 8
  %2 = load %class.CRectangle** %1
  %3 = bitcast %class.CRectangle* %2 to %class.CPolygon*
  call void @_ZN8CPolygonC2Ev(%class.CPolygon* %3) #3
  %4 = bitcast %class.CRectangle* %2 to i8***
  store i8** getelementptr inbounds ([3 x i8*]* @_ZTV10CRectangle, i64 0, i64 2), i8*** %4
  ret void
}

; Function Attrs: nounwind ssp uwtable
define linkonce_odr i32 @_ZN10CRectangle4areaEv(%class.CRectangle* %this) unnamed_addr #2 align 2 {
  %1 = alloca %class.CRectangle*, align 8
  store %class.CRectangle* %this, %class.CRectangle** %1, align 8
  %2 = load %class.CRectangle** %1
  %3 = bitcast %class.CRectangle* %2 to %class.CPolygon*
  %4 = getelementptr inbounds %class.CPolygon* %3, i32 0, i32 1
  %5 = load i32* %4, align 4
  %6 = bitcast %class.CRectangle* %2 to %class.CPolygon*
  %7 = getelementptr inbounds %class.CPolygon* %6, i32 0, i32 2
  %8 = load i32* %7, align 4
  %9 = mul nsw i32 %5, %8
  ret i32 %9
}

attributes #0 = { ssp uwtable }
attributes #1 = { inlinehint nounwind ssp uwtable }
attributes #2 = { nounwind ssp uwtable }
attributes #3 = { nounwind }
attributes #4 = { alwaysinline inlinehint ssp uwtable }
attributes #5 = { noinline noreturn nounwind }
attributes #6 = { noreturn nounwind }

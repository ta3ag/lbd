; ModuleID = 'ch_c++_2.cpp'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.9.0"

%"class.std::__1::allocator" = type { i8 }
%"class.std::__1::__libcpp_compressed_pair_imp" = type { %"struct.std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> >::__rep" }
%"struct.std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> >::__rep" = type { %union.anon }
%union.anon = type { %"struct.std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> >::__long" }
%"struct.std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> >::__long" = type { i64, i64, i8* }
%"class.std::__1::__compressed_pair" = type { %"class.std::__1::__libcpp_compressed_pair_imp" }
%"class.std::__1::basic_string" = type { %"class.std::__1::__compressed_pair" }
%"class.std::__1::__basic_string_common" = type { i8 }

@.str = private unnamed_addr constant [6 x i8] c"hello\00", align 1

define i32 @main() ssp uwtable {
  %1 = alloca i8*, align 8
  %2 = alloca %"class.std::__1::allocator"*, align 8
  %3 = alloca %"class.std::__1::__libcpp_compressed_pair_imp"*, align 8
  %4 = alloca %"class.std::__1::__compressed_pair"*, align 8
  %5 = alloca %"class.std::__1::__compressed_pair"*, align 8
  %6 = alloca %"class.std::__1::basic_string"*, align 8
  %7 = alloca i8*, align 8
  %8 = alloca %"class.std::__1::basic_string"*, align 8
  %9 = alloca i8*, align 8
  %10 = alloca i32, align 4
  %s = alloca %"class.std::__1::basic_string", align 8
  %11 = alloca i8*
  %12 = alloca i32
  store i32 0, i32* %10
  store %"class.std::__1::basic_string"* %s, %"class.std::__1::basic_string"** %8, align 8
  store i8* getelementptr inbounds ([6 x i8]* @.str, i32 0, i32 0), i8** %9, align 8
  %13 = load %"class.std::__1::basic_string"** %8
  %14 = load i8** %9, align 8
  store %"class.std::__1::basic_string"* %13, %"class.std::__1::basic_string"** %6, align 8
  store i8* %14, i8** %7, align 8
  %15 = load %"class.std::__1::basic_string"** %6
  %16 = bitcast %"class.std::__1::basic_string"* %15 to %"class.std::__1::__basic_string_common"*
  %17 = getelementptr inbounds %"class.std::__1::basic_string"* %15, i32 0, i32 0
  store %"class.std::__1::__compressed_pair"* %17, %"class.std::__1::__compressed_pair"** %5, align 8
  %18 = load %"class.std::__1::__compressed_pair"** %5
  store %"class.std::__1::__compressed_pair"* %18, %"class.std::__1::__compressed_pair"** %4, align 8
  %19 = load %"class.std::__1::__compressed_pair"** %4
  %20 = bitcast %"class.std::__1::__compressed_pair"* %19 to %"class.std::__1::__libcpp_compressed_pair_imp"*
  store %"class.std::__1::__libcpp_compressed_pair_imp"* %20, %"class.std::__1::__libcpp_compressed_pair_imp"** %3, align 8
  %21 = load %"class.std::__1::__libcpp_compressed_pair_imp"** %3
  %22 = bitcast %"class.std::__1::__libcpp_compressed_pair_imp"* %21 to %"class.std::__1::allocator"*
  store %"class.std::__1::allocator"* %22, %"class.std::__1::allocator"** %2, align 8
  %23 = load %"class.std::__1::allocator"** %2
  %24 = getelementptr inbounds %"class.std::__1::__libcpp_compressed_pair_imp"* %21, i32 0, i32 0
  %25 = load i8** %7, align 8
  %26 = load i8** %7, align 8
  store i8* %26, i8** %1, align 8
  %27 = load i8** %1, align 8
  %28 = invoke i64 @strlen(i8* %27)
          to label %.noexc unwind label %31

.noexc:                                           ; preds = %0
  invoke void @_ZNSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEE6__initEPKcm(%"class.std::__1::basic_string"* %15, i8* %25, i64 %28)
          to label %_ZNSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEC1EPKc.exit unwind label %31

_ZNSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEC1EPKc.exit: ; preds = %.noexc
  br label %29

; <label>:29                                      ; preds = %_ZNSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEC1EPKc.exit
  invoke void @_ZNSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEED1Ev(%"class.std::__1::basic_string"* %s)
          to label %30 unwind label %31

; <label>:30                                      ; preds = %29
  br label %38

; <label>:31                                      ; preds = %.noexc, %0, %29
  %32 = landingpad { i8*, i32 } personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*)
          catch i8* null
  %33 = extractvalue { i8*, i32 } %32, 0
  store i8* %33, i8** %11
  %34 = extractvalue { i8*, i32 } %32, 1
  store i32 %34, i32* %12
  br label %35

; <label>:35                                      ; preds = %31
  %36 = load i8** %11
  %37 = call i8* @__cxa_begin_catch(i8* %36) nounwind
  call void @__cxa_end_catch()
  br label %38

; <label>:38                                      ; preds = %35, %30
  %39 = load i32* %10
  ret i32 %39
}

declare i32 @__gxx_personality_v0(...)

declare void @_ZNSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEED1Ev(%"class.std::__1::basic_string"*)

declare i8* @__cxa_begin_catch(i8*)

declare void @__cxa_end_catch()

declare void @_ZNSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEE6__initEPKcm(%"class.std::__1::basic_string"*, i8*, i64)

declare i64 @strlen(i8*)

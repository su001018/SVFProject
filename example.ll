; ModuleID = 'example.c'
source_filename = "example.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 !dbg !8 {
entry:
  %retval = alloca i32, align 4
  %buf = alloca [7 x i8], align 1
  %ptr1 = alloca i8*, align 8
  %ptr2 = alloca i8*, align 8
  store i32 0, i32* %retval, align 4
  call void @llvm.dbg.declare(metadata [7 x i8]* %buf, metadata !13, metadata !DIExpression()), !dbg !18
  %arraydecay = getelementptr inbounds [7 x i8], [7 x i8]* %buf, i64 0, i64 0, !dbg !19
  %call = call i64 @read(i32 noundef 0, i8* noundef %arraydecay, i64 noundef 7), !dbg !20
  call void @llvm.dbg.declare(metadata i8** %ptr1, metadata !21, metadata !DIExpression()), !dbg !23
  %call1 = call noalias i8* @malloc(i64 noundef 8) #4, !dbg !24
  store i8* %call1, i8** %ptr1, align 8, !dbg !23
  call void @llvm.dbg.declare(metadata i8** %ptr2, metadata !25, metadata !DIExpression()), !dbg !26
  %call2 = call noalias i8* @malloc(i64 noundef 8) #4, !dbg !27
  store i8* %call2, i8** %ptr2, align 8, !dbg !26
  %arrayidx = getelementptr inbounds [7 x i8], [7 x i8]* %buf, i64 0, i64 5, !dbg !28
  %0 = load i8, i8* %arrayidx, align 1, !dbg !28
  %conv = sext i8 %0 to i32, !dbg !28
  %cmp = icmp eq i32 %conv, 101, !dbg !30
  br i1 %cmp, label %if.then, label %if.end, !dbg !31

if.then:                                          ; preds = %entry
  %1 = load i8*, i8** %ptr1, align 8, !dbg !32
  store i8* %1, i8** %ptr2, align 8, !dbg !34
  br label %if.end, !dbg !35

if.end:                                           ; preds = %if.then, %entry
  %arrayidx4 = getelementptr inbounds [7 x i8], [7 x i8]* %buf, i64 0, i64 3, !dbg !36
  %2 = load i8, i8* %arrayidx4, align 1, !dbg !36
  %conv5 = sext i8 %2 to i32, !dbg !36
  %cmp6 = icmp eq i32 %conv5, 115, !dbg !38
  br i1 %cmp6, label %if.then8, label %if.end15, !dbg !39

if.then8:                                         ; preds = %if.end
  %arrayidx9 = getelementptr inbounds [7 x i8], [7 x i8]* %buf, i64 0, i64 1, !dbg !40
  %3 = load i8, i8* %arrayidx9, align 1, !dbg !40
  %conv10 = sext i8 %3 to i32, !dbg !40
  %cmp11 = icmp eq i32 %conv10, 117, !dbg !43
  br i1 %cmp11, label %if.then13, label %if.end14, !dbg !44

if.then13:                                        ; preds = %if.then8
  %4 = load i8*, i8** %ptr1, align 8, !dbg !45
  call void @free(i8* noundef %4) #4, !dbg !47
  br label %if.end14, !dbg !48

if.end14:                                         ; preds = %if.then13, %if.then8
  br label %if.end15, !dbg !49

if.end15:                                         ; preds = %if.end14, %if.end
  %arrayidx16 = getelementptr inbounds [7 x i8], [7 x i8]* %buf, i64 0, i64 4, !dbg !50
  %5 = load i8, i8* %arrayidx16, align 1, !dbg !50
  %conv17 = sext i8 %5 to i32, !dbg !50
  %cmp18 = icmp eq i32 %conv17, 101, !dbg !52
  br i1 %cmp18, label %if.then20, label %if.end34, !dbg !53

if.then20:                                        ; preds = %if.end15
  %arrayidx21 = getelementptr inbounds [7 x i8], [7 x i8]* %buf, i64 0, i64 2, !dbg !54
  %6 = load i8, i8* %arrayidx21, align 1, !dbg !54
  %conv22 = sext i8 %6 to i32, !dbg !54
  %cmp23 = icmp eq i32 %conv22, 114, !dbg !56
  br i1 %cmp23, label %if.then25, label %if.end33, !dbg !57

if.then25:                                        ; preds = %if.then20
  %arrayidx26 = getelementptr inbounds [7 x i8], [7 x i8]* %buf, i64 0, i64 0, !dbg !58
  %7 = load i8, i8* %arrayidx26, align 1, !dbg !58
  %conv27 = sext i8 %7 to i32, !dbg !58
  %cmp28 = icmp eq i32 %conv27, 102, !dbg !60
  br i1 %cmp28, label %if.then30, label %if.end32, !dbg !61

if.then30:                                        ; preds = %if.then25
  %8 = load i8*, i8** %ptr2, align 8, !dbg !62
  %arrayidx31 = getelementptr inbounds i8, i8* %8, i64 0, !dbg !62
  store i8 109, i8* %arrayidx31, align 1, !dbg !63
  br label %if.end32, !dbg !62

if.end32:                                         ; preds = %if.then30, %if.then25
  br label %if.end33, !dbg !64

if.end33:                                         ; preds = %if.end32, %if.then20
  br label %if.end34, !dbg !65

if.end34:                                         ; preds = %if.end33, %if.end15
  ret i32 0, !dbg !66
}

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

declare dso_local i64 @read(i32 noundef, i8* noundef, i64 noundef) #2

; Function Attrs: nounwind
declare dso_local noalias i8* @malloc(i64 noundef) #3

; Function Attrs: nounwind
declare dso_local void @free(i8* noundef) #3

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nofree nosync nounwind readnone speculatable willreturn }
attributes #2 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { nounwind "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5, !6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 14.0.0", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "example.c", directory: "/home/test/workspace/code/pointerAnalysis", checksumkind: CSK_MD5, checksum: "3536620ef4061b15d471dc8d4ec4c175")
!2 = !{i32 7, !"Dwarf Version", i32 5}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 7, !"uwtable", i32 1}
!6 = !{i32 7, !"frame-pointer", i32 2}
!7 = !{!"clang version 14.0.0"}
!8 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 5, type: !9, scopeLine: 6, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !12)
!9 = !DISubroutineType(types: !10)
!10 = !{!11}
!11 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!12 = !{}
!13 = !DILocalVariable(name: "buf", scope: !8, file: !1, line: 7, type: !14)
!14 = !DICompositeType(tag: DW_TAG_array_type, baseType: !15, size: 56, elements: !16)
!15 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!16 = !{!17}
!17 = !DISubrange(count: 7)
!18 = !DILocation(line: 7, column: 10, scope: !8)
!19 = !DILocation(line: 8, column: 13, scope: !8)
!20 = !DILocation(line: 8, column: 5, scope: !8)
!21 = !DILocalVariable(name: "ptr1", scope: !8, file: !1, line: 9, type: !22)
!22 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !15, size: 64)
!23 = !DILocation(line: 9, column: 11, scope: !8)
!24 = !DILocation(line: 9, column: 18, scope: !8)
!25 = !DILocalVariable(name: "ptr2", scope: !8, file: !1, line: 10, type: !22)
!26 = !DILocation(line: 10, column: 11, scope: !8)
!27 = !DILocation(line: 10, column: 18, scope: !8)
!28 = !DILocation(line: 11, column: 9, scope: !29)
!29 = distinct !DILexicalBlock(scope: !8, file: !1, line: 11, column: 9)
!30 = !DILocation(line: 11, column: 16, scope: !29)
!31 = !DILocation(line: 11, column: 9, scope: !8)
!32 = !DILocation(line: 13, column: 16, scope: !33)
!33 = distinct !DILexicalBlock(scope: !29, file: !1, line: 12, column: 5)
!34 = !DILocation(line: 13, column: 14, scope: !33)
!35 = !DILocation(line: 14, column: 5, scope: !33)
!36 = !DILocation(line: 15, column: 9, scope: !37)
!37 = distinct !DILexicalBlock(scope: !8, file: !1, line: 15, column: 9)
!38 = !DILocation(line: 15, column: 16, scope: !37)
!39 = !DILocation(line: 15, column: 9, scope: !8)
!40 = !DILocation(line: 17, column: 13, scope: !41)
!41 = distinct !DILexicalBlock(scope: !42, file: !1, line: 17, column: 13)
!42 = distinct !DILexicalBlock(scope: !37, file: !1, line: 16, column: 5)
!43 = !DILocation(line: 17, column: 20, scope: !41)
!44 = !DILocation(line: 17, column: 13, scope: !42)
!45 = !DILocation(line: 19, column: 18, scope: !46)
!46 = distinct !DILexicalBlock(scope: !41, file: !1, line: 18, column: 9)
!47 = !DILocation(line: 19, column: 13, scope: !46)
!48 = !DILocation(line: 20, column: 9, scope: !46)
!49 = !DILocation(line: 21, column: 5, scope: !42)
!50 = !DILocation(line: 22, column: 9, scope: !51)
!51 = distinct !DILexicalBlock(scope: !8, file: !1, line: 22, column: 9)
!52 = !DILocation(line: 22, column: 16, scope: !51)
!53 = !DILocation(line: 22, column: 9, scope: !8)
!54 = !DILocation(line: 23, column: 13, scope: !55)
!55 = distinct !DILexicalBlock(scope: !51, file: !1, line: 23, column: 13)
!56 = !DILocation(line: 23, column: 20, scope: !55)
!57 = !DILocation(line: 23, column: 13, scope: !51)
!58 = !DILocation(line: 24, column: 17, scope: !59)
!59 = distinct !DILexicalBlock(scope: !55, file: !1, line: 24, column: 17)
!60 = !DILocation(line: 24, column: 24, scope: !59)
!61 = !DILocation(line: 24, column: 17, scope: !55)
!62 = !DILocation(line: 25, column: 17, scope: !59)
!63 = !DILocation(line: 25, column: 25, scope: !59)
!64 = !DILocation(line: 24, column: 27, scope: !59)
!65 = !DILocation(line: 23, column: 23, scope: !55)
!66 = !DILocation(line: 26, column: 5, scope: !8)

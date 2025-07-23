; ModuleID = 'trigger'
source_filename = "trigger"

define void @main() {
entry:
  call void @llvm.tsl.trigger.pos(i32 90)
  %sig_4 = load i32, ptr inttoptr (i32 4 to ptr), align 4
  %cmptmp = icmp eq i32 %sig_4, 268553420
  br i1 %cmptmp, label %then, label %else

then:                                             ; preds = %entry
  call void @llvm.tsl.trigger()
  br label %merge

else:                                             ; preds = %entry
  br label %merge

merge:                                            ; preds = %else, %then
  call void @inst1()
  ret void
}

define internal void @inst1() {
entry:
  br label %s1

s1:                                               ; preds = %entry
  %sig_4104 = load i32, ptr inttoptr (i32 4104 to ptr), align 4
  %0 = call i32 @llvm.tsl.bit.slice(i32 %sig_4104, i32 5, i32 5)
  %1 = call i1 @llvm.tsl.edge.detect(i32 %0, i32 1)
  br i1 %1, label %then, label %else

s2:                                               ; preds = %else
  %sig_4 = load i32, ptr inttoptr (i32 4 to ptr), align 4
  %sig_4096 = load i32, ptr inttoptr (i32 4096 to ptr), align 4
  %mathtmp = xor i32 %sig_4, %sig_4096
  %2 = call i32 @llvm.tsl.bit.slice(i32 %mathtmp, i32 3, i32 0)
  %cmptmp = icmp sge i32 %2, 13
  br i1 %cmptmp, label %then1, label %else2

s3:                                               ; preds = %then1
  %3 = call i1 @llvm.tsl.edge.detect(i32 %sig_4104, i32 0)
  br i1 %3, label %then4, label %else5

then:                                             ; preds = %s1
  call void @llvm.tsl.display(i32 1)
  br label %merge

else:                                             ; preds = %s1
  br label %s2

merge:                                            ; preds = %then
  ret void

then1:                                            ; preds = %s2
  call void @llvm.tsl.display(i32 2)
  br label %s3

else2:                                            ; preds = %s2
  br label %merge3

merge3:                                           ; preds = %else2
  ret void

then4:                                            ; preds = %s3
  call void @llvm.tsl.trigger()
  br label %merge10

else5:                                            ; preds = %s3
  %sig_4100 = load i32, ptr inttoptr (i32 4100 to ptr), align 4
  %mathtmp6 = and i32 %sig_4096, %sig_4100
  %4 = call i32 @llvm.tsl.bit.slice(i32 %mathtmp6, i32 1, i32 1)
  %5 = call i1 @llvm.tsl.edge.detect(i32 %4, i32 1)
  br i1 %5, label %then7, label %else8

then7:                                            ; preds = %else5
  call void @llvm.tsl.trigger()
  br label %merge9

else8:                                            ; preds = %else5
  call void @llvm.tsl.display(i32 3)
  br label %merge9

merge9:                                           ; preds = %else8, %then7
  ret void

merge10:                                          ; preds = %then4
  ret void
}

; Function Attrs: nounwind
declare void @llvm.tsl.trigger.pos(i32) #0

; Function Attrs: nounwind
declare void @llvm.tsl.trigger() #0

; Function Attrs: nounwind willreturn
declare i32 @llvm.tsl.bit.slice(i32, i32, i32) #1

; Function Attrs: nounwind willreturn
declare i1 @llvm.tsl.edge.detect(i32, i32) #1

; Function Attrs: nounwind
declare void @llvm.tsl.display(i32) #0

attributes #0 = { nounwind }
attributes #1 = { nounwind willreturn }

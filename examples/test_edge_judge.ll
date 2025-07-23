; ModuleID = 'trigger'
source_filename = "trigger"

define void @main() {
entry:
  call void @llvm.tsl.trigger.pos(i32 50)
  call void @inst1()
  ret void
}

define internal void @inst1() {
entry:
  br label %s1

s1:                                               ; preds = %entry
  br label %s2

s2:                                               ; preds = %s1
  %load_lhs = load i32, ptr inttoptr (i32 4096 to ptr), align 4
  %mathtmp = xor i32 %load_lhs, 9
  %cmptmp = icmp sge i32 %mathtmp, 13
  br i1 %cmptmp, label %then, label %else

s3:                                               ; preds = %then
  %load_lhs1 = load i32, ptr inttoptr (i32 4100 to ptr), align 4
  %cmptmp2 = icmp eq i32 %load_lhs1, -1431655766
  br i1 %cmptmp2, label %then3, label %else4

then:                                             ; preds = %s2
  br label %s3

else:                                             ; preds = %s2
  br label %merge

merge:                                            ; preds = %else
  ret void

then3:                                            ; preds = %s3
  call void @llvm.tsl.trigger()
  br label %merge5

else4:                                            ; preds = %s3
  br label %merge5

merge5:                                           ; preds = %else4, %then3
  ret void
}

; Function Attrs: nounwind
declare void @llvm.tsl.trigger.pos(i32) #0

; Function Attrs: nounwind
declare void @llvm.tsl.trigger() #0

attributes #0 = { nounwind }

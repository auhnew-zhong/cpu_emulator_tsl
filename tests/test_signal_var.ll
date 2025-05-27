; ModuleID = 'trigger'
source_filename = "trigger"


define void @main() {
entry:
  call void @llvm.tsl.trigger.pos(i32 60)
  call void @inst1()
  ret void
}

define internal void @inst1() {
entry:
  br label %s1

s1:                                               ; preds = %entry
  br label %s2

s2:
  %addr = inttoptr i32 1024 to ptr                                               ; preds = %s1
  %load_lhs = load i32, ptr %addr, align 4
  %mathtmp = and i32 %load_lhs, 13
  %cmptmp = icmp sge i32 %mathtmp, 9
  br i1 %cmptmp, label %then, label %else

then:                                             ; preds = %s2
  call void @llvm.tsl.trigger()
  br label %merge

else:                                             ; preds = %s2
  br label %merge

merge:                                            ; preds = %else, %then
  ret void
}

; Function Attrs: nounwind
declare void @llvm.tsl.trigger.pos(i32) #0

; Function Attrs: nounwind
declare void @llvm.tsl.trigger() #0

attributes #0 = { nounwind }


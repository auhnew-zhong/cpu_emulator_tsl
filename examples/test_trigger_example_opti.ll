; ModuleID = 'trigger'
source_filename = "trigger"

; Function Attrs: nounwind
define void @main() local_unnamed_addr #0 {
entry:
  tail call void @llvm.tsl.trigger.pos(i32 90)
  %sig_4 = load i32, ptr inttoptr (i32 4 to ptr), align 4
  %cmptmp = icmp eq i32 %sig_4, 268553420
  br i1 %cmptmp, label %then, label %merge

then:                                             ; preds = %entry
  tail call void @llvm.tsl.trigger()
  br label %merge

merge:                                            ; preds = %entry, %then
  %sig_4096.i = load i32, ptr inttoptr (i32 4096 to ptr), align 4096
  %cmptmp.i = icmp eq i32 %sig_4096.i, 268439825
  br i1 %cmptmp.i, label %then.i, label %inst1.exit

then.i:                                           ; preds = %merge
  %sig_4.i = load i32, ptr inttoptr (i32 4 to ptr), align 4
  %mathtmp.i = xor i32 %sig_4.i, 268439825
  %cmptmp1.i = icmp sgt i32 %mathtmp.i, 12
  br i1 %cmptmp1.i, label %then2.i, label %inst1.exit

then2.i:                                          ; preds = %then.i
  %sig_4104.i = load i32, ptr inttoptr (i32 4104 to ptr), align 8
  %cmptmp5.i = icmp eq i32 %sig_4104.i, -1431655766
  br i1 %cmptmp5.i, label %then6.i, label %inst1.exit

then6.i:                                          ; preds = %then2.i
  tail call void @llvm.tsl.trigger()
  br label %inst1.exit

inst1.exit:                                       ; preds = %merge, %then.i, %then2.i, %then6.i
  ret void
}

; Function Attrs: nounwind
declare void @llvm.tsl.trigger.pos(i32) #0

; Function Attrs: nounwind
declare void @llvm.tsl.trigger() #0

attributes #0 = { nounwind }

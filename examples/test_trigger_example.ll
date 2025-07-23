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
  %sig_4096 = load i32, ptr inttoptr (i32 4096 to ptr), align 4
  %cmptmp = icmp eq i32 %sig_4096, 268439825
  br i1 %cmptmp, label %then, label %else

s2:                                               ; preds = %then
  %sig_4 = load i32, ptr inttoptr (i32 4 to ptr), align 4
  %mathtmp = xor i32 %sig_4, %sig_4096
  %cmptmp1 = icmp sge i32 %mathtmp, 13
  br i1 %cmptmp1, label %then2, label %else3

s3:                                               ; preds = %then2
  %sig_4104 = load i32, ptr inttoptr (i32 4104 to ptr), align 4
  %cmptmp5 = icmp eq i32 %sig_4104, -1431655766
  br i1 %cmptmp5, label %then6, label %else7

then:                                             ; preds = %s1
  br label %s2

else:                                             ; preds = %s1
  br label %merge

merge:                                            ; preds = %else
  ret void

then2:                                            ; preds = %s2
  br label %s3

else3:                                            ; preds = %s2
  br label %merge4

merge4:                                           ; preds = %else3
  ret void

then6:                                            ; preds = %s3
  call void @llvm.tsl.trigger()
  br label %merge13

else7:                                            ; preds = %s3
  %sig_4100 = load i32, ptr inttoptr (i32 4100 to ptr), align 4
  %mathtmp8 = and i32 %sig_4096, %sig_4100
  %cmptmp9 = icmp eq i32 %mathtmp8, 15
  br i1 %cmptmp9, label %then10, label %else11

then10:                                           ; preds = %else7
  call void @llvm.tsl.trigger()
  br label %merge12

else11:                                           ; preds = %else7
  br label %merge12

merge12:                                          ; preds = %else11, %then10
  ret void

merge13:                                          ; preds = %then6
  ret void
}

; Function Attrs: nounwind
declare void @llvm.tsl.trigger.pos(i32) #0

; Function Attrs: nounwind
declare void @llvm.tsl.trigger() #0

attributes #0 = { nounwind }

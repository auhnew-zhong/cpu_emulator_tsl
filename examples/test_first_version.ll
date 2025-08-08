; ModuleID = 'trigger'
source_filename = "trigger"

@counter0 = global i32 10
@counter1 = global i32 12

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
  call void @llvm.tsl.domain(i32 0)
  %sig_4104 = load i32, ptr inttoptr (i32 4104 to ptr), align 4
  %0 = call i32 @llvm.tsl.bit.slice(i32 %sig_4104, i32 5, i32 5)
  %1 = call i1 @llvm.tsl.edge.detect(i32 %0, i32 1)
  br i1 %1, label %then, label %else

s2:                                               ; preds = %else
  call void @llvm.tsl.domain(i32 0)
  %counter0_load1 = load i32, ptr @counter0, align 4
  %counter0_dec = sub i32 %counter0_load1, 1
  store i32 %counter0_dec, ptr @counter0, align 4
  %sig_4 = load i32, ptr inttoptr (i32 4 to ptr), align 4
  %sig_4096 = load i32, ptr inttoptr (i32 4096 to ptr), align 4
  %mathtmp = xor i32 %sig_4, %sig_4096
  %2 = call i32 @llvm.tsl.bit.slice(i32 %mathtmp, i32 3, i32 0)
  %cmptmp = icmp sge i32 %2, 13
  br i1 %cmptmp, label %then2, label %else3

s3:                                               ; preds = %then2
  %counter0_load9 = load i32, ptr @counter0, align 4
  store i32 0, ptr @counter0, align 4
  %counter1_load = load i32, ptr @counter1, align 4
  store i32 0, ptr @counter1, align 4
  %3 = call i1 @llvm.tsl.edge.detect(i32 %sig_4104, i32 0)
  br i1 %3, label %then10, label %else11

then:                                             ; preds = %s1
  %counter0_load = load i32, ptr @counter0, align 4
  %counter0_inc = add i32 %counter0_load, 1
  store i32 %counter0_inc, ptr @counter0, align 4
  call void @llvm.tsl.timer(i32 0, i32 0)
  call void @llvm.tsl.display(i32 0)
  br label %merge

else:                                             ; preds = %s1
  br label %s2

merge:                                            ; preds = %then
  ret void

then2:                                            ; preds = %s2
  call void @llvm.tsl.timer(i32 0, i32 1)
  call void @llvm.tsl.display(i32 1)
  br label %s3

else3:                                            ; preds = %s2
  br label %merge4

merge4:                                           ; preds = %else3
  %load_left = load i32, ptr @counter1, align 4
  %cmptmp5 = icmp eq i32 %load_left, 15
  br i1 %cmptmp5, label %then6, label %else7

then6:                                            ; preds = %merge4
  call void @llvm.tsl.trigger()
  br label %merge8

else7:                                            ; preds = %merge4
  br label %merge8

merge8:                                           ; preds = %else7, %then6
  ret void

then10:                                           ; preds = %s3
  call void @llvm.tsl.trigger()
  br label %merge16

else11:                                           ; preds = %s3
  %sig_4100 = load i32, ptr inttoptr (i32 4100 to ptr), align 4
  %mathtmp12 = and i32 %sig_4096, %sig_4100
  %4 = call i32 @llvm.tsl.bit.slice(i32 %mathtmp12, i32 1, i32 1)
  %5 = call i1 @llvm.tsl.edge.detect(i32 %4, i32 1)
  br i1 %5, label %then13, label %else14

then13:                                           ; preds = %else11
  call void @llvm.tsl.trigger()
  br label %merge15

else14:                                           ; preds = %else11
  call void @llvm.tsl.display(i32 2)
  br label %merge15

merge15:                                          ; preds = %else14, %then13
  br label %merge16

merge16:                                          ; preds = %merge15, %then10
  ret void
}

; Function Attrs: nounwind
declare void @llvm.tsl.trigger.pos(i32) #0

; Function Attrs: nounwind
declare void @llvm.tsl.trigger() #0

; Function Attrs: nounwind
declare void @llvm.tsl.domain(i32) #0

; Function Attrs: nounwind willreturn
declare i32 @llvm.tsl.bit.slice(i32, i32, i32) #1

; Function Attrs: nounwind willreturn
declare i1 @llvm.tsl.edge.detect(i32, i32) #1

; Function Attrs: nounwind
declare void @llvm.tsl.timer(i32, i32) #0

; Function Attrs: nounwind
declare void @llvm.tsl.display(i32) #0

attributes #0 = { nounwind }
attributes #1 = { nounwind willreturn }

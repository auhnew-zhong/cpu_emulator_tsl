; ModuleID = 'trigger'
source_filename = "trigger"

@counter0 = global i32 10
@counter1 = global i32 12

define void @main() {
entry:
  call void @llvm.tsl.timer(i32 0, i32 3, i32 -294967296, ptr blockaddress(@inst1, %s2))
  call void @llvm.tsl.trigger.pos(i32 10)
  %top.ctl.sig0.w0 = load i32, ptr inttoptr (i32 4 to ptr), align 4
  %cmptmp = icmp eq i32 %top.ctl.sig0.w0, 268553420
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
  %top.op.w0 = load i32, ptr inttoptr (i32 4104 to ptr), align 4
  %0 = call i32 @llvm.tsl.bit.slice(i32 %top.op.w0, i32 5, i32 5)
  %1 = call i1 @llvm.tsl.edge.detect(i32 %0, i32 1)
  br i1 %1, label %then, label %else

s2:                                               ; preds = %else
  call void @llvm.tsl.domain(i32 1)
  %op.ctl.sig0.w0 = load i32, ptr inttoptr (i32 4 to ptr), align 4
  %cmptmp = icmp eq i32 %op.ctl.sig0.w0, 1
  br i1 %cmptmp, label %then1, label %else2

then:                                             ; preds = %s1
  %counter0_load = load i32, ptr @counter0, align 4
  %counter0_inc = add i32 %counter0_load, 1
  store i32 %counter0_inc, ptr @counter0, align 4
  call void @llvm.tsl.timer(i32 0, i32 1, i32 -294967296, ptr blockaddress(@inst1, %s2))
  call void @llvm.tsl.send(i32 0, i32 0, i32 0)
  call void @llvm.tsl.send(i32 1, i32 1, i32 0)
  br label %merge

else:                                             ; preds = %s1
  call void @llvm.tsl.send(i32 1, i32 2, i32 0)
  call void @llvm.tsl.send(i32 1, i32 3, i32 0)
  call void @llvm.tsl.send(i32 1, i32 4, i32 0)
  call void @llvm.tsl.send(i32 1, i32 5, i32 0)
  call void @llvm.tsl.send(i32 0, i32 6, i32 0)
  br label %s2

merge:                                            ; preds = %then
  ret void

then1:                                            ; preds = %s2
  call void @llvm.tsl.send(i32 1, i32 7, i32 0)
  call void @llvm.tsl.trigger()
  br label %merge3

else2:                                            ; preds = %s2
  call void @llvm.tsl.send(i32 1, i32 8, i32 0)
  call void @llvm.tsl.send(i32 1, i32 9, i32 0)
  br label %merge3

merge3:                                           ; preds = %else2, %then1
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
declare void @llvm.tsl.timer(i32, i32, i32, ptr) #0

; Function Attrs: nounwind
declare void @llvm.tsl.send(i32, i32, i32) #0

attributes #0 = { nounwind }
attributes #1 = { nounwind willreturn }

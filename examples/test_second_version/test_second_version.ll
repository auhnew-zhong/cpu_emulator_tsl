; ModuleID = 'trigger'
source_filename = "trigger"

@counter0 = global i32 10
@counter1 = global i32 12

define void @main() {
entry:
  call void @llvm.tsl.timer(i32 0, i32 3, i32 -294967296, ptr blockaddress(@inst1, %s2))
  call void @llvm.tsl.trigger.pos(i32 90)
  %top.ctl.sig0 = load i32, ptr inttoptr (i32 4 to ptr), align 4
  %cmptmp = icmp eq i32 %top.ctl.sig0, 268553420
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
  %top.op = load i32, ptr inttoptr (i32 4104 to ptr), align 4
  %0 = call i32 @llvm.tsl.bit.slice(i32 %top.op, i32 5, i32 5)
  %1 = call i1 @llvm.tsl.edge.detect(i32 %0, i32 1)
  br i1 %1, label %then, label %else

s2:                                               ; preds = %else
  call void @llvm.tsl.domain(i32 1)
  %counter0_load1 = load i32, ptr @counter0, align 4
  %counter0_dec = sub i32 %counter0_load1, 1
  store i32 %counter0_dec, ptr @counter0, align 4
  %top.ctl.sig0 = load i32, ptr inttoptr (i32 4 to ptr), align 4
  %top.ctl.sig1 = load i32, ptr inttoptr (i32 4096 to ptr), align 4
  %xortmp = xor i32 %top.ctl.sig0, %top.ctl.sig1
  %2 = call i32 @llvm.tsl.bit.slice(i32 %top.ctl.sig0, i32 3, i32 0)
  %3 = call i32 @llvm.tsl.bit.slice(i32 %top.ctl.sig1, i32 3, i32 0)
  %concat_result = call i32 (...) @llvm.tsl.concat(i32 %2, i32 %3)
  call void @llvm.tsl.display(i32 1)
  %isunknown_result = call i1 @llvm.tsl.isunknown(i32 %top.ctl.sig0)
  %isunknown_result2 = call i1 @llvm.tsl.isunknown(i32 %xortmp)
  %isunknown_result3 = call i1 @llvm.tsl.isunknown(i32 %concat_result)
  call void @llvm.tsl.display(i32 2)
  call void @llvm.tsl.display(i32 3)
  call void @llvm.tsl.display(i32 4)
  %4 = call i32 @llvm.tsl.bit.slice(i32 %xortmp, i32 3, i32 0)
  %cmptmp = icmp sge i32 %4, 13
  %load_left = load i32, ptr @counter0, align 4
  %cmptmp4 = icmp eq i32 %load_left, 15
  %lognottmp = xor i1 %cmptmp4, true
  %logandtmp = and i1 %cmptmp, %lognottmp
  br i1 %logandtmp, label %then5, label %else6

s3:                                               ; preds = %else12
  call void @llvm.tsl.domain(i32 2)
  %counter0_load15 = load i32, ptr @counter0, align 4
  store i32 0, ptr @counter0, align 4
  %counter1_load = load i32, ptr @counter1, align 4
  store i32 0, ptr @counter1, align 4
  %5 = call i32 @llvm.tsl.bit.slice(i32 %top.op, i32 1, i32 0)
  %6 = call i32 @llvm.tsl.bit.slice(i32 %top.ctl.sig1, i32 1, i32 0)
  %top.ctl.sig2 = load i32, ptr inttoptr (i32 4100 to ptr), align 4
  %7 = call i32 @llvm.tsl.bit.slice(i32 %top.ctl.sig2, i32 1, i32 0)
  %concat_result16 = call i32 (...) @llvm.tsl.concat(i32 %6, i32 %7)
  %concat_result17 = call i32 (...) @llvm.tsl.concat(i32 %5, i32 %concat_result16)
  call void @llvm.tsl.display(i32 6)
  %isunknown_result18 = call i1 @llvm.tsl.isunknown(i32 %top.op)
  %isunknown_result19 = call i1 @llvm.tsl.isunknown(i32 %concat_result17)
  call void @llvm.tsl.display(i32 7)
  call void @llvm.tsl.display(i32 8)
  %8 = call i32 @llvm.tsl.bit.slice(i32 %top.op, i32 3, i32 0)
  %bit_3 = call i32 @llvm.tsl.bit.slice(i32 %8, i32 3, i32 3)
  %9 = call i1 @llvm.tsl.edge.detect(i32 %bit_3, i32 0)
  %bit_1 = call i32 @llvm.tsl.bit.slice(i32 %8, i32 1, i32 1)
  %10 = call i1 @llvm.tsl.edge.detect(i32 %bit_1, i32 2)
  %bit_0 = call i32 @llvm.tsl.bit.slice(i32 %8, i32 0, i32 0)
  %11 = call i1 @llvm.tsl.edge.detect(i32 %bit_0, i32 4)
  %multibit_edge_and = and i1 %9, true
  %multibit_edge_and20 = and i1 %multibit_edge_and, %10
  %multibit_edge_and21 = and i1 %multibit_edge_and20, %11
  br i1 %multibit_edge_and21, label %then22, label %else23

s4:                                               ; preds = %else27
  call void @llvm.tsl.domain(i32 0)
  %12 = call i32 @llvm.tsl.bit.slice(i32 %top.ctl.sig0, i32 7, i32 0)
  %reduction_xor_result = call i1 @llvm.tsl.reduction(i32 %12, i32 2)
  call void @llvm.tsl.display(i32 12)
  %13 = call i32 @llvm.tsl.bit.slice(i32 %top.ctl.sig1, i32 3, i32 0)
  %reduction_and_result = call i1 @llvm.tsl.reduction(i32 %13, i32 0)
  call void @llvm.tsl.display(i32 13)
  %14 = call i32 @llvm.tsl.bit.slice(i32 %top.ctl.sig2, i32 7, i32 0)
  %reduction_or_result = call i1 @llvm.tsl.reduction(i32 %14, i32 1)
  call void @llvm.tsl.display(i32 14)
  %15 = call i32 @llvm.tsl.bit.slice(i32 %top.ctl.sig0, i32 7, i32 0)
  %bitwise_not_tmp = xor i32 %15, -1
  call void @llvm.tsl.display(i32 15)
  %cast_left = zext i1 %reduction_xor_result to i32
  %cmptmp30 = icmp eq i32 %cast_left, 1
  br i1 %cmptmp30, label %then31, label %else32

then:                                             ; preds = %s1
  %counter0_load = load i32, ptr @counter0, align 4
  %counter0_inc = add i32 %counter0_load, 1
  store i32 %counter0_inc, ptr @counter0, align 4
  call void @llvm.tsl.timer(i32 0, i32 0, i32 -294967296, ptr blockaddress(@inst1, %s2))
  call void @llvm.tsl.display(i32 0)
  call void @llvm.tsl.exec(i32 0)
  br label %merge

else:                                             ; preds = %s1
  br label %s2

merge:                                            ; preds = %then
  ret void

then5:                                            ; preds = %s2
  call void @llvm.tsl.timer(i32 0, i32 1, i32 -294967296, ptr blockaddress(@inst1, %s2))
  call void @llvm.tsl.display(i32 5)
  call void @llvm.tsl.exec(i32 1)
  br label %merge14

else6:                                            ; preds = %s2
  %load_left7 = load i32, ptr @counter1, align 4
  %cmptmp8 = icmp eq i32 %load_left7, 15
  %load_left9 = load i32, ptr @counter0, align 4
  %cmptmp10 = icmp eq i32 %load_left9, 15
  %logortmp = or i1 %cmptmp8, %cmptmp10
  br i1 %logortmp, label %then11, label %else12

then11:                                           ; preds = %else6
  call void @llvm.tsl.trigger()
  br label %merge13

else12:                                           ; preds = %else6
  br label %s3

merge13:                                          ; preds = %then11
  br label %merge14

merge14:                                          ; preds = %merge13, %then5
  ret void

then22:                                           ; preds = %s3
  call void @llvm.tsl.trigger()
  br label %merge29

else23:                                           ; preds = %s3
  %andtmp = and i32 %top.ctl.sig1, %top.ctl.sig2
  %16 = call i32 @llvm.tsl.bit.slice(i32 %andtmp, i32 0, i32 0)
  %17 = call i32 @llvm.tsl.bit.slice(i32 %top.op, i32 0, i32 0)
  %concat_result24 = call i32 (...) @llvm.tsl.concat(i32 %16, i32 %17)
  %isunknown_result25 = call i1 @llvm.tsl.isunknown(i32 %concat_result24)
  call void @llvm.tsl.display(i32 9)
  call void @llvm.tsl.display(i32 10)
  %18 = call i32 @llvm.tsl.bit.slice(i32 %andtmp, i32 1, i32 1)
  %19 = call i1 @llvm.tsl.edge.detect(i32 %18, i32 1)
  br i1 %19, label %then26, label %else27

then26:                                           ; preds = %else23
  call void @llvm.tsl.trigger()
  br label %merge28

else27:                                           ; preds = %else23
  call void @llvm.tsl.display(i32 11)
  br label %s4

merge28:                                          ; preds = %then26
  br label %merge29

merge29:                                          ; preds = %merge28, %then22
  ret void

then31:                                           ; preds = %s4
  call void @llvm.tsl.display(i32 16)
  call void @llvm.tsl.exec(i32 2)
  br label %merge38

else32:                                           ; preds = %s4
  %cast_left33 = zext i1 %reduction_and_result to i32
  %cmptmp34 = icmp eq i32 %cast_left33, 0
  br i1 %cmptmp34, label %then35, label %else36

then35:                                           ; preds = %else32
  call void @llvm.tsl.display(i32 17)
  call void @llvm.tsl.trigger()
  br label %merge37

else36:                                           ; preds = %else32
  call void @llvm.tsl.display(i32 18)
  call void @llvm.tsl.trigger()
  br label %merge37

merge37:                                          ; preds = %else36, %then35
  br label %merge38

merge38:                                          ; preds = %merge37, %then31
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
declare void @llvm.tsl.display(i32) #0

; Function Attrs: nounwind
declare void @llvm.tsl.exec(i32) #0

; Function Attrs: nounwind willreturn
declare i32 @llvm.tsl.concat(...) #1

; Function Attrs: nounwind willreturn
declare i1 @llvm.tsl.isunknown(i32) #1

; Function Attrs: nounwind willreturn
declare i1 @llvm.tsl.reduction(i32, i32) #1

attributes #0 = { nounwind }
attributes #1 = { nounwind willreturn }

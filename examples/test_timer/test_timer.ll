; ModuleID = 'trigger'
source_filename = "trigger"

@counter0 = global i32 10
@counter1 = global i32 12

define void @main() {
entry:
  call void @llvm.tsl.trigger.pos(i32 90)                                         ; preds = %s2
  call void @llvm.tsl.timer(i32 0, i32 1)
  call void @llvm.tsl.display(i32 5)
  call void @llvm.tsl.exec(i32 1)                                 ; preds = %merge37, %then31
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

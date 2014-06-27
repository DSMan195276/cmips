sll $0, $0, 0
srl $0, $0, 0
sra $0, $0, 0

sllv $0, $0, $0
srlv $0, $0, $0
srav $0, $0, $0

jr $0
jalr $0

add $0, $0, $0
addu $0, $0, $0
sub $0, $0, $0
subu $0, $0, $0
and $0, $0, $0
or $0, $0, $0
xor $0, $0, $0
nor $0, $0, $0
slt $0, $0, $0
sltu $0, $0, $0

syscall
break

j 0
jal 0

beq $0, $0, 0
bne $0, $0, 0

addi $0, $0, 0
addiu $0, $0, 0
slti $0, $0, 0
sltiu $0, $0, 0
andi $0, $0, 0
ori $0, $0, 0
xori $0, $0, 0

lui $0, 0

lb $0, 0($0)
lh $0, 0($0)
lwl $0, 0($0)
lw $0, 0($0)
lbu $0, 0($0)
lhu $0, 0($0)
lwr $0, 0($0)
sb $0, 0($0)
sh $0, 0($0)
swl $0, 0($0)
sw $0, 0($0)
swr $0, 0($0)

nop
noop


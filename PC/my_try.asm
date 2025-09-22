#####################################################################
#C语言冒泡排序源码
#####################################################################
# int i,j;
# int swaaped;
# for(i=0;i<7;i++){
# swapped=0;for(j=0;j<n-1-i;j++){
# if(arr[j]>arr[j+1]){
# arr[j]^=arr[j+1];arr[j+1]^=arr[j];arr[j]^=arr[j+1];swaped=1;}}
# if(swapped==0) break;    
# }
#####################################################################
# registers used status:
# mem[0x180], student id.
# mem[0x184], sorted student id
#
# x11  the size of Sid
# x12  record max index
# x15  the Sid
#####################################################################
        addi x2,x0,0x02
        slli x2,x2,8
        addi x2,x2,0x18
        slli x2,x2,16
        addi x3,x0,0x12
        slli x3,x3,8
        addi x3,x3,0x52
        add  x2,x2,x3
        sw   x2,0x180(x0)
        addi x10,x0,8           # the size of Sid
        lw   x15,0x180(x0)
        addi x4,x0,0x0f
	addi x11,x10,-1
	
out_loop:
        addi x2,x0,0

in_loop:
        slli x9,x2,2
        slli x6,x2,2
        addi x6,x6,4

        srl  x7,x15,x9          #x7>>(4*i)
        and  x7,x7,x4
        srl  x8,x15,x6          #arr[i+1]
        and  x8,x8,x4
        slt  x14,x7,x8
        beq  x14,x0,skip_swap   #�?末尾数字�?大时，跳过交�?

swap:   
        sll  x7,x7,x9           #x7<<(4*i)
        sll  x8,x8,x6           #x8<<(4*(i+1))

        xori x7,x7,-1
        xori x8,x8,-1

        and  x15,x15,x7         #clear
        and  x15,x15,x8         #clear

        xori x7,x7,-1
        xori x8,x8,-1

        slli x7,x7,4
        srli x8,x8,4

        or x15,x15,x7
        or x15,x15,x8

skip_swap:
        addi x2,x2,1
        bne  x2,x11,in_loop

        addi x11,x11,-1
        bne  x11,x0,out_loop

        sw   x15,0x184(x0)

end:    lui  x2,0xffff0
        ori  x1,x2,0x004
        ori  x2,x2,0x00c
        addi x5,x0,0x300
        andi x5,x5,0x100
        jal  x0,end

        
        

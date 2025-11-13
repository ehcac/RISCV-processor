.global main
.text
main:
    addi x1, x0, 5      
    addi x2, x0, 10       
    addi x3, x0, 0       

    lw   x4, 0(x1)     
    sw   x2, 4(x1)      

    slt  x5, x1, x2      
    sll  x6, x2, x1      
    slli x7, x1, 2        

    beq  x1, x2, LABEL1 
    blt  x1, x2, LABEL2 

LABEL1:
    addi x8, x0, 1       
    jal x0, END

LABEL2:
    addi x8, x0, 2    

END:
    addi x10, x0, 0      

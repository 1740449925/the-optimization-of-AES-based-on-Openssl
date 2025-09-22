module Forward(
    input        MEM_MemWrite,
    input  [4:0] MEM_rd,
    input  [4:0] EX_rs1,
    input  [4:0] EX_rs2,
    input        WB_RegWrite,
    input  [4:0] WB_rd,
    input        MEM_RegWrite,
    input  [4:0] MEM_rs2,
    output [1:0] Forward_A_OP,
    output [1:0] Forward_B_OP,
    output       Disrc
);

//ForAOP
 assign Forward_A_OP[1] = MEM_RegWrite && (MEM_rd == EX_rs1); 
 assign Forward_A_OP[0] = WB_RegWrite  && (WB_rd  == EX_rs1) && ~Forward_A_OP[1];
//ForBop
 assign Forward_B_OP[1] = MEM_RegWrite  &&(MEM_rd == EX_rs2); 
 assign Forward_B_OP[0] = WB_RegWrite   &&(WB_rd  == EX_rs2) && ~Forward_B_OP[1];
//Disrc
 assign Disrc = WB_RegWrite && (WB_rd == MEM_rs2) && MEM_MemWrite;

endmodule
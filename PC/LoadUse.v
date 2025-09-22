module LoadUse(
input       EX_MemRead,
input [4:0] EX_rd,
input [4:0] rs1,
input [4:0] rs2,
output      Load_use_Flag
);

    assign Load_use_Flag = EX_MemRead && ((EX_rd==rs1) || (EX_rd==rs2));

endmodule
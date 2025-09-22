`include "ctrl_encode_def.v"

module NPC(PC, NPCOp, IMM, NPC,rs1,EX_pc);  // next pc module
   input  [31:0] PC;        // pc
   input  [4:0]  NPCOp;     // next pc operation
   input  [31:0] IMM;       // immediate
   input  [31:0] rs1;      //RD1
   input  [31:0] EX_pc;
   output reg [31:0] NPC;   // next pc
   
   wire [31:0] PCPLUS4;
   assign PCPLUS4 = PC + 4; // pc + 4
  
   always @(*) begin
        case (NPCOp)//SEPC是异常时PC寄存器
            `NPC_JALR:   NPC = rs1+IMM;   //rs1+imm
            `NPC_PLUS4:  NPC = PCPLUS4;
            `NPC_BRANCH: NPC = EX_pc+IMM;    //B type, NPC computes addr
            `NPC_JUMP:   NPC = EX_pc+IMM;    //J type, NPC computes addr
            default:     NPC = PCPLUS4;
        endcase
    end // end always
   
endmodule

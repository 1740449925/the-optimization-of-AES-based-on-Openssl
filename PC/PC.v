module PC( clk, rst, NPC, PC,Load_use_Flag );
  input              clk;
  input              rst;
  input       [31:0] NPC;
  input              Load_use_Flag;
  output reg  [31:0] PC;

  always @(posedge clk, posedge rst) begin
    if (rst) begin
       PC <= 32'h0000_0000;
       //$write("\n reset pc = %h: ", PC);
       end
    else if(Load_use_Flag)
    begin
      PC <= PC;
    end
    else
       begin 
         PC <= NPC; 
         //$write("\n pc = %h: ", PC);
       end
  end
  
endmodule


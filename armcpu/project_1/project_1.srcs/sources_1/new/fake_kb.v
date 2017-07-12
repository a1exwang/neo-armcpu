`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 07/12/2017 01:58:43 PM
// Design Name: 
// Module Name: fake_kb
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module fake_kb(
    input clk,
	input rst,
	input int_ack,
	input key,
	output reg int_req,
	output reg [7:0] kbd_ascii
    );

    // maintain int
    always @(posedge clk) begin
        if (int_req && int_ack)
            int_req <= 0;
        if (rst) begin
            kbd_ascii <= 0;
            int_req <= 0;
        end else if (!int_req && key) begin
            int_req <= 1'b1;
            kbd_ascii <= 8'h41;
        end
    end
endmodule

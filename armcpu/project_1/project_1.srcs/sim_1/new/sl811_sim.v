`timescale 1ns / 100ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 07/15/2017 10:34:42 PM
// Design Name: 
// Module Name: sl811_sim
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


module sl811_sim(
    output wire[7:0] raw_data_out,
    output wire[7:0] data_out,
    output wire raw_a0,
    output wire raw_we_n,
    output wire raw_rd_n,
    output wire raw_cs_n,
    output wire raw_rst_n
    );
    
    reg clk = 0, rst = 1, 
        rw = 1, we = 1, oe = 0;
    reg [7:0] data_in = 8'h0;
    // oe = 1 for write to sl811
    wire [7:0] raw_data;
    assign raw_data = oe ? 8'h32 : 8'bZZZZZZZZ;
    assign raw_data_out = oe ? 8'bZZZZZZZZ : raw_data;
    
    sl811 sl811_test(
        .clk50M(clk),
        .rst(rst),
        .data_in(data_in),
        .data_out(data_out),
        .rw(rw), // 0 for read, 1 for write
        .we(we), // More precisely 'operation enabled'
        .ce(0),
        .raw_a0(raw_a0),
        .raw_data(raw_data),
        .raw_we_n(raw_we_n),
        .raw_rd_n(raw_rd_n),
        .raw_cs_n(raw_cs_n),
        .raw_rst_n(raw_rst_n));

    always #10 clk = ~clk;
    initial begin
        $timeformat(-9,3,"ns",12);
    end
    initial begin
        we = 1;
        rw = 1;
        data_in = 8'h0;
        oe = 0;
        
        rst = 1;
        #100;
        rst = 0;
        #100;
        
        // write addr
        oe = 0;
        rw <= 1;
        we <= 0;
        data_in <= 8'h88;
        #20;
        
        we = 1;
        #20;
        
        // write data
        rw <= 1;
        we <= 0;
        data_in <= 8'hFC;
        #20;
        we = 1;
        #20;
        
        // read data
        oe = 1;
        rw = 0;
        we = 0;
        #20;
        we = 1;
        #20;
        
    end
    


endmodule

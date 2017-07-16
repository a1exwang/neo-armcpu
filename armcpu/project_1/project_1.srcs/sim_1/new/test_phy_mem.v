`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 07/16/2017 12:07:15 AM
// Design Name: 
// Module Name: test_phy_mem
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
`define SL811_CTRL_ADDR	32'h0F000000	// {24'b0, sl811_reg}
`define SL811_DATA_ADDR	32'h0F000004// {24'b0, sl811_read_data}


module test_phy_mem(
    output [31:0] data_out_mem,
    output busy,
    output sl811_we,
    output sl811_rw,
    output sl811_ce,
    
    output [7:0] raw_data_out,
    output raw_a0,
    output raw_we_n,
    output raw_rd_n,
    output raw_cs_n,
    output raw_rst_n
    );
    reg clk = 0, rst = 0, is_write = 0;
    reg oe = 0;
    reg [31:0] addr;
    reg [31:0] data_in_mem;
    wire [7:0] data_sl2mem;
    wire [7:0] data_mem2sl;
    
    phy_mem_ctrl phy_mem_test(
        .clk50M(clk),
        .rst(rst),
    
        .is_write(is_write),
        .addr(addr),
        .data_in(data_in_mem),
        .data_out(data_out_mem),
        .busy(busy),
       
        // sl811 interface
        .sl811_data_in(data_sl2mem),
        .sl811_data_out(data_mem2sl),
        .sl811_we(sl811_we),
        .sl811_rw(sl811_rw),
        .sl811_ce(sl811_ce)
        );
    wire [7:0] raw_data;
    sl811 sl811_test(
            .clk50M(clk),
            .rst(rst),
            .data_in(data_mem2sl),
            .data_out(data_sl2mem),
            .rw(sl811_rw), // 0 for read, 1 for write
            .we(sl811_we), // More precisely 'operation enabled'
            .ce(sl811_ce),
            .raw_data(raw_data),
            .raw_a0(raw_a0),
            .raw_we_n(raw_we_n),
            .raw_rd_n(raw_rd_n),
            .raw_cs_n(raw_cs_n),
            .raw_rst_n(raw_rst_n));
    assign raw_data = oe ? 8'h44 : 8'bZZZZZZZZ;
    assign raw_data_out = oe ? 8'bZZZZZZZZ : raw_data;
    always #10 clk = ~clk;
        initial begin
            $timeformat(-9,3,"ns",12);
        end
        initial begin
            is_write = 0;
            addr = 32'h0;
            data_in_mem = 32'h0;
            
            rst = 1;
            #100;
            rst = 0;
            #100;
            
            // write addr
            is_write = 1;
            addr = `SL811_CTRL_ADDR;
            data_in_mem = 32'h1ab;
            #280;
            
            is_write = 0;
            #20;
            
            // write data
            is_write = 1;
            addr = `SL811_CTRL_ADDR;
            data_in_mem = 32'hcd;
            #280;
            is_write = 0;
            #20;
            
//            // read data
//            is_write = 1;
//            addr = `SL811_CTRL_ADDR;
//            data_in_mem = 32'h2e;
//            #155;
//            oe = 1;
//            #165;
            
//            is_write = 0;
//            addr = `SL811_DATA_ADDR;
//            #320;
        end
endmodule

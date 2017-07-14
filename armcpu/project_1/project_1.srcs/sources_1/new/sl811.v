`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 07/14/2017 12:37:57 PM
// Design Name: 
// Module Name: sl811
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


module sl811(
    input clk50M,
    input rst,
    input [7:0] addr,
    input [7:0] data_in,
    output wire[7:0] data_out,
    input rw, // 0 for read, 1 for write
    input we, // More precisely 'operation enabled'
    input ce,
    inout wire[7:0] raw_data,
    output reg raw_a0,
    output reg raw_we_n,
    output reg raw_rd_n,
    output wire raw_cs_n,
    output wire raw_rst_n);
    
    parameter NEXT_READ = 0, NEXT_WRITE = 1,
        NEXT_ADDR = 0, NEXT_DATA = 1;
    
    reg [7:0] counter = 8'h41;
    reg next_op = NEXT_READ;
    reg addr_or_data = NEXT_ADDR;

    reg [7:0] raw_data_out;
    reg writing_to_sl811;
    reg read_flag_we = 0, read_flag_clk = 0;
    
    always @(posedge we) begin
        read_flag_we = ~read_flag_we;
        if (addr_or_data == NEXT_ADDR) begin
            // Currently writing address
            if (rw == 0) begin
                // Next read
                next_op = NEXT_READ;
                addr_or_data = NEXT_ADDR;
            end else begin
                next_op = NEXT_WRITE;
                addr_or_data = NEXT_DATA;
            end
        end else begin
            // Currently writing data
            counter = data_in;
            addr_or_data = NEXT_ADDR;
        end
    end
    
    reg [7:0] data_out_buf;
    reg [3:0] state;
    reg read_in_this_cycle_done;
    parameter WAIT = 4'h0, READING = 4'h1, READING_SAVE = 4'h2, READING_RECOVER = 4'h3;
    
    always @(posedge clk50M) begin
        if (rst) begin
            state = 4'h0;
        end else begin
            case (state)
                WAIT: begin
                    // Waiting
                    if (read_flag_we != read_flag_clk) begin
                        // Write addr
                        raw_data_out = data_in;
                        writing_to_sl811 = 1;
                        raw_we_n = 0;
                        raw_rd_n = 1;
                        state = READING;
                        read_flag_clk = read_flag_we;
                    end

                    writing_to_sl811 = 0;
                    raw_we_n = 1;
                    raw_rd_n = 1;
                    state = WAIT;
                end
                READING: begin
                    // Read data
                    writing_to_sl811 = 0;
                    raw_we_n = 1;
                    raw_rd_n = 0;
                    state = READING_SAVE;
                end
                READING_SAVE: begin
                    // Wait for data to be stable
                    data_out_buf = raw_data;
                    state = WAIT;
                end
            endcase           
        end
    end
    
    assign raw_cs_n = 0;
    assign raw_rst_n = 1;
    assign raw_data = writing_to_sl811 ? raw_data_out : 8'bZZZZZZZZ;
    assign data_out = data_out_buf;
    
endmodule

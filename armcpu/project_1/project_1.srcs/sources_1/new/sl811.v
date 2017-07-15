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
    reg addr_or_data = NEXT_ADDR;

    reg [7:0] raw_data_out;
    reg writing_to_sl811;
    reg action_flag_we = 0, action_flag_clk = 0;
    reg action;
    parameter READ = 0, WRITE = 1;
    
    always @(negedge we) begin
        action_flag_we = ~action_flag_we;
        if (addr_or_data == NEXT_ADDR) begin
            // Currently writing address
            if (rw == 0) begin
                // Next read
                addr_or_data = NEXT_ADDR;
                action = READ;
            end else begin
                addr_or_data = NEXT_DATA;
                action = WRITE;
            end
        end else begin // NEXT_DATA
            // Currently writing data
            counter = data_in;
            addr_or_data = NEXT_ADDR;
            action = WRITE;
        end
    end
    
    reg [7:0] data_out_buf;
    reg [3:0] state;
    reg read_in_this_cycle_done;
    parameter WAIT = 4'h0, READING = 4'h1, READING_SAVE = 4'h2, 
        READING_RECOVER = 4'h3, BEFORE_READING = 4'h4;
    
    reg [3:0] cnt_before_reading = 0;
    
    always @(posedge clk50M) begin
        if (rst) begin
            state = 4'h0;
            data_out_buf = 8'h0;
        end else begin
            case (state)
                WAIT: begin
                    writing_to_sl811 = 0;
                    raw_we_n = 1;
                    raw_rd_n = 1;
                    state = WAIT;
                    // Waiting
                    if (action_flag_we != action_flag_clk) begin
                        raw_data_out = data_in;
                        writing_to_sl811 = 1;
                        raw_we_n = 0;
                        raw_rd_n = 1;
                        
                        action_flag_clk = action_flag_we;
                        if (action == READ) begin
                            // Write addr for read op
                            state = BEFORE_READING;
                            raw_a0 = 0;
                        end else begin
                            // Write addr/data for write op
                            state = WAIT;
                            if (addr_or_data == NEXT_ADDR)
                                // Write data for write op
                                raw_a0 = 1;    
                            else
                                raw_a0 = 0;
                        end
                    end
                end
                BEFORE_READING: begin
                    if (cnt_before_reading == 5) begin
                        cnt_before_reading = 0;
                        state = READING;
                    end else begin
                        cnt_before_reading = cnt_before_reading + 1;
                    end
                end
                READING: begin
                    // Read data
                    writing_to_sl811 = 0;
                    raw_we_n = 1;
                    raw_rd_n = 0;
                    raw_a0 = 1;
                    data_out_buf = raw_data;
                    state = READING_SAVE;
                end
                READING_SAVE: begin
                    // Wait for data to be stable
                    data_out_buf = raw_data;
                    raw_a0 = 1;
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

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
    parameter WAIT = 4'h0, 
        READING = 4'h1, READING_SAVE = 4'h2, 
        READING_RECOVER = 4'h3, 
        WRITING_ADDR_FOR_READ = 4'h4, WRITE_ADDR_HOLD_A0_DATA = 4'h5,
        WRITING_ADDR_FOR_READ_WR_HOLD = 4'h6,
        
        WRITING_HOLD_FOR_WRITE_ADDR = 4'h7, WRITING_HOLD_FOR_WRITE_A0 = 4'h8,
        WRITING_ADDR_RECOVERY = 4'h9,
        
        WRITING_HOLD_FOR_WRITE_DATA = 4'hA,
        WRITING_HOLD_FOR_WRITE_DATA_D = 4'hB,
        WRITING_DATA_RECOVERY = 4'hC;
    
    reg [3:0] cnt_wafr = 0;
    reg [3:0] cnt_reading = 0;
    reg [3:0] cnt_reading_recover = 0;
    reg [3:0] cnt_hold = 0;
    reg [3:0] cnt_writing_a0 = 0;
    reg [3:0] cnt_before_reading1 = 0;
    reg [3:0] cnt_wafrwh = 0;
    
    always @(posedge clk50M) begin
        if (rst) begin
            state = 4'h0;
            data_out_buf = 8'h32;
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
                            state = WRITING_ADDR_FOR_READ;
                            raw_a0 = 0;
                        end else begin
                            if (addr_or_data == NEXT_ADDR) begin
                                // Write data for write op
                                state = WRITING_HOLD_FOR_WRITE_DATA;
                            end else begin
                                // Write addr for write op 
                                raw_a0 = 0;   
                                state = WRITING_HOLD_FOR_WRITE_ADDR;
                            end
                        end
                    end
                end
                WRITING_HOLD_FOR_WRITE_ADDR: begin
                    if (cnt_hold == 3) begin
                        state = WRITING_HOLD_FOR_WRITE_A0;
                        cnt_hold = 0;
                    end else begin
                        cnt_hold = cnt_hold + 1;
                    end
                end
                WRITING_HOLD_FOR_WRITE_A0: begin
                    raw_we_n = 1;
                    state = WRITING_ADDR_RECOVERY;
                end
                WRITING_ADDR_RECOVERY: begin
                    state = WAIT;
                    raw_a0 = 1;
                    writing_to_sl811 = 0;
                end
                
                WRITING_HOLD_FOR_WRITE_DATA: begin
                    if (cnt_hold == 3) begin
                        state = WRITING_HOLD_FOR_WRITE_DATA_D;
                        cnt_hold = 0;
                    end else begin
                        cnt_hold = cnt_hold + 1;
                    end
                end
                WRITING_HOLD_FOR_WRITE_DATA_D: begin
                    raw_we_n = 1;
                    state = WRITING_DATA_RECOVERY;
                end
                WRITING_DATA_RECOVERY: begin
                    state = WAIT;
                    writing_to_sl811 = 0;
                end
                
                WRITING_ADDR_FOR_READ: begin // wait for nWR 0 holds, 100ns
                    if (cnt_wafr == 3) begin
                        cnt_wafr = 0;
                        state = WRITE_ADDR_HOLD_A0_DATA;
                    end else begin
                        cnt_wafr = cnt_wafr + 1;
                    end
                end
                WRITE_ADDR_HOLD_A0_DATA: begin // wait for A0 and Data, 20ns
                    raw_we_n = 1;
                    state = WRITING_ADDR_FOR_READ_WR_HOLD;
                end
                WRITING_ADDR_FOR_READ_WR_HOLD: begin // wait for nWR 1 holds, 80ns
                    raw_a0 = 1;
                    writing_to_sl811 = 0;
                    if (cnt_wafrwh == 3) begin
                        cnt_wafrwh = 0;
                        state = READING;
                    end else begin
                        cnt_wafrwh = cnt_wafrwh + 1;
                    end
                end
                READING: begin // Waiting for data to stablize, 40ns
                    // Read data
                    raw_rd_n = 0;
                    if (cnt_reading == 1) begin
                        state = READING_SAVE;
                        cnt_reading = 0;
                    end else begin
                        cnt_reading = cnt_reading + 1;
                    end
                end
                READING_SAVE: begin // Read data, 20ns
                    // Wait for data to be stable
                    data_out_buf = raw_data;
                    state = READING_RECOVER;
                end
                READING_RECOVER: begin // Waiting for rd = 1 holds, 40ns
                    if (cnt_reading_recover == 1) begin
                        cnt_reading_recover = 0;
                        state = WAIT;
                    end else begin
                        cnt_reading_recover = cnt_reading_recover + 1;
                    end
                end
            endcase           
        end
    end
    
    assign raw_cs_n = ce;
    assign raw_rst_n = ~rst;
    assign raw_data = writing_to_sl811 ? raw_data_out : 8'bZZZZZZZZ;
    assign data_out = data_out_buf;
    
endmodule

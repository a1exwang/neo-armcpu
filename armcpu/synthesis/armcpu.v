/*
 * $File: armcpu.v
 * $Date: Fri Dec 20 15:23:08 2013 +0800
 * $Author: jiakai <jia.kai66@gmail.com>
 */

module armcpu(
	input clk50M,
	input rst_key,
	input clk_manual,
	output [0:6] segdisp0,
	output [0:6] segdisp1,

	output reg [15:0] led,
	input [31:0] params,

	// ram interface
	output [19:0] baseram_addr,
	inout [31:0] baseram_data,
	output baseram_ce,
	output baseram_oe,
	output baseram_we,
	output [19:0] extram_addr,
	inout [31:0] extram_data,
	output extram_ce,
	output extram_oe,
	output extram_we,

	// serial port interface
	output com_TxD,
	input com_RxD,

	// flash interface
	output [22:0] flash_addr,
	inout [15:0] flash_data,
	output [7:0] flash_ctl,

	// VGA interface
	output [8:0] vga_color_out,
	output vga_hsync,
	output vga_vsync,
	output vga_de,

	input kbd_enb_hi,
	input kbd_enb_lo,
	input [3:0] kbd_data,
	output [31:0] test_mmu_instr_addr,
	input key,
	
	inout [7:0] sl811_raw_data,
    output sl811_a0,
    output sl811_we_n,
    output sl811_rd_n,
    output sl811_cs_n,
    output sl811_rst_n,
    input sl811_int);

	// ------------------------------------------------------------------

	assign rst = ~rst_key;


	reg clk_cpu;
	assign single_step_mode = params[25];
	wire [24:0] cpu_speed = params[24:0];
	wire [4:0] monitor_data_shift = params[30:26];
	assign rom_selector = params[31];
	reg rom_selector_prev, set_rst_by_rom_selector;
	reg [24:0] clk50M_cnt;

	wire kbd_int_ack, kbd_int_req;
	wire [7:0] kbd_ascii;

	reg clk_manual_prev;
	assign clk_manual_posedge = !clk_manual_prev && clk_manual;
	always @(posedge clk50M)
		clk_manual_prev <= clk_manual;

	always @(posedge clk50M) begin
		if ((!single_step_mode && clk50M_cnt >= cpu_speed) ||
				(single_step_mode && clk_manual_posedge)) begin
			clk50M_cnt <= 0;
			clk_cpu <= ~clk_cpu;
		end else
			clk50M_cnt <= clk50M_cnt + 1'b1;
	end

	always @(posedge clk_cpu) begin
		rom_selector_prev <= rom_selector;
		set_rst_by_rom_selector <= (rom_selector != rom_selector_prev);
	end

	reg [7:0] monitor_data;
	wire [31:0] segdisp_data;
	assign write_protect = 0;	//baseram_addr <= 512;  // XXX: write-protect for code
	wire baseram_we_set;
	assign baseram_we = baseram_we_set | write_protect;

	system usys(.clk_cpu(clk_cpu), .clk50M(clk50M),
		.rst(rst || set_rst_by_rom_selector),

		.segdisp(segdisp_data),

		.rom_selector(rom_selector),

		.baseram_addr(baseram_addr),
		.baseram_data(baseram_data),
		.baseram_ce(baseram_ce),
		.baseram_oe(baseram_oe),	
		.baseram_we(baseram_we_set),
		.extram_addr(extram_addr),
		.extram_data(extram_data),
		.extram_ce(extram_ce),
		.extram_oe(extram_oe),
		.extram_we(extram_we),

		.com_TxD(com_TxD),
		.com_RxD(com_RxD),
	
		.flash_addr(flash_addr),
		.flash_data(flash_data),
		.flash_ctl(flash_ctl),
	
		.vga_color_out(vga_color_out),
		.vga_hsync(vga_hsync),
		.vga_vsync(vga_vsync),
		.vga_de(vga_de),
	
		.kbd_int(kbd_int_req),
		.kbd_int_ack(kbd_int_ack),
		.kbd_data(kbd_ascii),
		.test_mmu_instr_addr(test_mmu_instr_addr),
		
		.sl811_raw_data(sl811_raw_data),
        .sl811_a0(sl811_a0),
        .sl811_we_n(sl811_we_n),
        .sl811_rd_n(sl811_rd_n),
        .sl811_cs_n(sl811_cs_n),
        .sl811_rst_n(sl811_rst_n),
        .sl811_int(sl811_int));

	always @(posedge clk_cpu)
		led[7:0] <= {cpu_speed[3:0], led[2:0], !led[2:0]};

	always @(posedge clk50M) begin
		// led[15:8] <= segdisp_data[7:0];
		monitor_data <= segdisp_data >> monitor_data_shift;
	end

	always @(*)
		led[15:8] <= {kbd_int_req, kbd_ascii[6:0]};

	digseg_driver useg0(.data(monitor_data[3:0]), .seg(segdisp0));
	digseg_driver useg1(.data(monitor_data[7:4]), .seg(segdisp1));

//	ps2_drv ups2drv(.clk(clk50M), .rst(rst),
//		.int_ack(kbd_int_ack),
//		.int_req(kbd_int_req),
//		.kbd_ascii(kbd_ascii),
//		.kbd_enb_hi(kbd_enb_hi),
//		.kbd_enb_lo(kbd_enb_lo),
//		.kbd_data(kbd_data));
	fake_kb ups2drv(.clk(clk50M), .rst(rst),
		.int_ack(kbd_int_ack),
		.int_req(kbd_int_req),
		.kbd_ascii(kbd_ascii),
		.key(key));

endmodule


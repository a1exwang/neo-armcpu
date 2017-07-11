`default_nettype wire

module thinpad_top(/*autoport*/
//inout
            base_ram_data,
            ext_ram_data,
            flash_data,
            sl811_data,
            dm9k_data,
//output
            base_ram_addr,
            base_ram_be_n,
            base_ram_ce_n,
            base_ram_oe_n,
            base_ram_we_n,
            ext_ram_addr,
            ext_ram_be_n,
            ext_ram_ce_n,
            ext_ram_oe_n,
            ext_ram_we_n,
            txd,
            flash_address,
            flash_rp_n,
            flash_vpen,
            flash_oe_n,
            flash_ce,
            flash_byte_n,
            flash_we_n,
            sl811_a0,
            sl811_we_n,
            sl811_rd_n,
            sl811_cs_n,
            sl811_rst_n,
            sl811_drq,
            dm9k_cmd,
            dm9k_we_n,
            dm9k_rd_n,
            dm9k_cs_n,
            dm9k_rst_n,
            leds,
            vga_pixel,
            vga_hsync,
            vga_vsync,
            vga_clk,
            vga_de,
//input
            clk_in,
            clk_uart_in,
            rxd,
            sl811_dack,
            sl811_int,
            dm9k_int,
            dip_sw,
            touch_btn);

input wire clk_in; //50MHz main clock input
input wire clk_uart_in; //11.0592MHz clock for UART

//Base memory signals
inout wire[31:0] base_ram_data;
output wire[19:0] base_ram_addr;
output wire[3:0] base_ram_be_n;
output wire base_ram_ce_n;
output wire base_ram_oe_n;
output wire base_ram_we_n;
assign base_ram_be_n=4'b0; //leave ByteEnable zero if you don't know what it is

//Extension memory signals
inout wire[31:0] ext_ram_data;
output wire[19:0] ext_ram_addr;
output wire[3:0] ext_ram_be_n;
output wire ext_ram_ce_n;
output wire ext_ram_oe_n;
output wire ext_ram_we_n;
assign ext_ram_be_n=4'b0;

//Serial port signals
output wire txd;
input wire rxd;

//Flash memory, JS28F640
output wire [21:0]flash_address;
output wire flash_rp_n;
output wire flash_vpen;
output wire flash_oe_n;
inout wire [15:0]flash_data;
output wire flash_ce;
output wire flash_byte_n;
output wire flash_we_n;

//SL811 USB controller signals
output wire sl811_a0;
inout wire[7:0] sl811_data;
output wire sl811_we_n;
output wire sl811_rd_n;
output wire sl811_cs_n;
output wire sl811_rst_n;
input wire sl811_dack;
input wire sl811_int;
output wire sl811_drq;

//DM9000 Ethernet controller signals
output wire dm9k_cmd;
inout wire[15:0] dm9k_data;
output wire dm9k_we_n;
output wire dm9k_rd_n;
output wire dm9k_cs_n;
output wire dm9k_rst_n;
input wire dm9k_int;

//LED, SegDisp, DIP SW, and BTN1~6
output wire[31:0] leds;
input wire[31:0] dip_sw;
input wire[5:0] touch_btn;

//Video output
output wire[7:0] vga_pixel;
output wire vga_hsync;
output wire vga_vsync;
output wire vga_clk;
output wire vga_de;

//////////////////////////////////////////////////////////
// -- Code starts here
/////////////////////////////////////////////////////////

// USB
assign sl811_a0 = 0;
assign sl811_we_n = 0;
assign sl811_rd_n = 0;
assign sl811_cs_n = 0;
assign sl811_rst_n = 0;
assign sl811_drq = 0;

//DM9000 Ethernet controller signals
assign dm9k_cmd = 0;
assign dm9k_we_n = 0;
assign dm9k_rd_n = 0;
assign dm9k_cs_n = 0;
assign dm9k_rst_n = 0;

// VGA
assign vga_clk = clk_in;

/////////////////// VGA Test
//localparam H_VISIBLE_AREA = 800,
//    H_FRONT_PORCH = 56,
//    H_SYNC_PULSE = 120,
//    H_BACK_PORCH = 64,
//    H_WHOLE = 1040;

//localparam V_VISIBLE_AREA = 600,
//    V_FRONT_PORCH = 37,
//    V_SYNC_PULSE = 6,
//    V_BACK_PORCH = 23,
//    V_WHOLE = 666;

//localparam RAM_ADDR_BASE = 18'h2000;
//// i do not know why lower addr of uram does not work

//reg [10:0] hsync_cnt = 0;
//reg [10:0] vsync_cnt = 0;

//assign vga_hsync = (hsync_cnt >= H_SYNC_PULSE);
//assign vga_vsync = (vsync_cnt >= V_SYNC_PULSE);

//wire [10:0] pixel_x = (hsync_cnt >= H_SYNC_PULSE + H_FRONT_PORCH ?
//    hsync_cnt - H_SYNC_PULSE - H_FRONT_PORCH : {11{1'b1}});
//wire [10:0] pixel_y = (vsync_cnt >= V_SYNC_PULSE + V_FRONT_PORCH ?
//    vsync_cnt - V_SYNC_PULSE - V_FRONT_PORCH : {11{1'b1}});
//wire [`VGA_ADDR_WIDTH-1:0] pixel_ram_addr =
//    {pixel_y[10:1], {`VGA_WIDTH_MULT_SHIFT{1'b0}}} + pixel_x[10:1];

//wire should_draw = pixel_x >= 0 && pixel_x < H_VISIBLE_AREA &&
//    pixel_y >= 0 && pixel_y < V_VISIBLE_AREA;

//wire [2:0] red = 3'b101, green = 3'b100, blue = 3'b111;
//reg [7:0] vga_color_out;
//reg vga_data_en1;
//always @(posedge clk_in) begin
//    if (hsync_cnt == H_WHOLE - 1) begin
//        hsync_cnt <= 0;
//        if (vsync_cnt == V_WHOLE - 1) begin
//            vsync_cnt <= 0;
//        end else begin
//            vsync_cnt <= vsync_cnt + 1'b1;
//        end
//    end else begin
//        hsync_cnt <= hsync_cnt + 1'b1;
//    end
//    if (should_draw) begin
//        vga_color_out <= {blue, green, red};
//        vga_data_en1 <= 1;
//    end else begin
//        vga_color_out <= 0;
//        vga_data_en1 <= 0;
//    end
//end
//assign vga_pixel = vga_color_out;
//assign vga_de = vga_data_en1 
wire [31:0] test_mmu_instr_addr;
assign leds[15:0] = test_mmu_instr_addr[15:0];

wire rst, clk_manual;
assign rst = ~touch_btn[4];
assign clk_manual = touch_btn[5];
armcpu wtfcpu(
    .clk50M(clk_in),
    .rst_key(rst),
    .clk_manual(clk_manual),
    .segdisp0({leds[23:22],leds[19:17],leds[20],leds[21]}),
    .segdisp1({leds[31:30],leds[27:25],leds[28],leds[29]}),
    
    //.led(leds[15:0]),
    .params(dip_sw),
    
    .baseram_addr(base_ram_addr),
    .baseram_data(base_ram_data),
    .baseram_ce(base_ram_ce_n),
    .baseram_oe(base_ram_oe_n),
    .baseram_we(base_ram_we_n),
    
    .extram_addr(ext_ram_addr),
    .extram_data(ext_ram_data),
    .extram_ce(ext_ram_ce_n),
    .extram_oe(ext_ram_oe_n),
    .extram_we(~ext_ram_we_n),
    
    .com_TxD(txd),
    .com_RxD(rxd),
    
    .flash_addr({0,flash_address}),
    .flash_data(flash_data),
    .flash_ctl({~flash_byte_n,flash_ce, 2'bZZ,
        ~flash_oe_n,~flash_rp_n,flash_vpen,~flash_we_n}),
  
    .vga_color_out(vga_pixel),
    .vga_hsync(vga_hsync),
    .vga_vsync(vga_vsync),
    .vga_de(vga_de),
    
    .kbd_enb_hi(0),
    .kbd_enb_lo(0),
    .kbd_data(0),
    .test_mmu_instr_addr(test_mmu_instr_addr)
);

endmodule

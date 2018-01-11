# Makefile borrowed from https://github.com/cliffordwolf/icestorm/blob/master/examples/icestick/Makefile
#
# The following license is from the icestorm project and specifically applies to this file only:
#
#  Permission to use, copy, modify, and/or distribute this software for any
#  purpose with or without fee is hereby granted, provided that the above
#  copyright notice and this permission notice appear in all copies.
#
#  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
#  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
#  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
#  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
#  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
#  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
#  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

TOP_MODULE = top
MODULES = top.v edge_detect.v usb_fs_in_arb.v usb_fs_in_pe.v usb_fs_mux.v usb_fs_out_arb.v usb_fs_out_pe.v usb_fs_pe.v usb_fs_rx.v usb_fs_tx.v usb_fs_tx_mux.v usb_serial_ctrl_ep.v usb_spi_bridge_ep.v

ESP_UPDUINO_IP := 10.238.37.113
SUBSECTORSIZE := 1024

PIN_DEF = pins.pcf
DEVICE_FAMILY=ice40up
DEVICE = 5k
DEVICE_PACKAGE=sg48

SHELL := /bin/bash


all: $(TOP_MODULE).rpt $(TOP_MODULE).bin

%.blif: $(MODULES)
	yosys -p 'synth_ice40 -top $(TOP_MODULE) -blif $@' $^

%.asc: $(PIN_DEF) %.blif
	arachne-pnr -d $(DEVICE) -P $(DEVICE_PACKAGE) -o $@ -p $^

%.bin: %.asc
	icepack $< $@

%.rpt: %.asc
	icetime -d $(subst ice40,,$(DEVICE_FAMILY))$(DEVICE) -mtr $@ $<

%_tb: %_tb.v %.v
	iverilog -o $@ $^

%_tb.vcd: %_tb
	vvp -N $< +vcd=$@

%_syn.v: %.blif
	yosys -p 'read_blif -wideports $^; write_verilog $@'

%_syntb: %_tb.v %_syn.v
	iverilog -o $@ $^ `yosys-config --datdir/ice40/cells_sim.v`

%_syntb.vcd: %_syntb
	vvp -N $< +vcd=$@

prog: $(TOP_MODULE).bin
	iceprog $<

sudo-prog: $(TOP_MODULE).bin
	@echo 'Executing prog as root!!!'
	sudo iceprog $<

clean:
	rm -f $(TOP_MODULE).blif $(TOP_MODULE).asc $(TOP_MODULE).rpt $(TOP_MODULE).bin

erase_esp_upduino: $(TOP_MODULE).bin
	{ \
	set -e ; \
	binsize=$$( wc -c $< ); \
	binsize=$${binsize%% *}; \
	subsectorstart=0; \
	while [ $$subsectorstart -lt $$binsize ]; \
	do \
	echo "erase subsector: $${subsectorstart}"; \
		curl -s "http://$(ESP_UPDUINO_IP)/upduino.lc?start=$${subsectorstart}&cmd=erase"; \
		subsectorstart=$$(( subsectorstart + $(SUBSECTORSIZE) )); \
	done; \
	}

write_esp_upduino: $(TOP_MODULE).bin
	{ \
	set -e ; \
	cat $< | od -tx1 -Ax -v -w128 | while read adr h; \
	do echo $$adr; \
		while [ $$(curl -s "http://$(ESP_UPDUINO_IP)/upduino.lc?cmd=status" | cut -c1-5) != "00:80" ]; \
		do echo "flash busy"; \
			sleep 0.5; \
		done; \
		curl -s "http://$(ESP_UPDUINO_IP)/upduino.lc?start=0x$${adr}&cmd=write&hexdata=$${h// /}"; \
	done \
	}

verify_esp_upduino: $(TOP_MODULE).bin
	{ \
	set -e ; \
	cat $< | od -tx1 -Ax -v -w128 | while read adr h; \
	do \
		eh=$$(curl -s "http://$(ESP_UPDUINO_IP)/upduino.lc?start=0x$${adr}&cmd=read&size=128"); \
		if [ "$${h// /}" == "$${eh}" ]; then \
			echo $${adr} ok; \
		else \
			echo "ERR: 0x$${adr}"; \
			exit 1; \
		fi; \
	done; \
	}

.SECONDARY:
.PHONY: all prog clean

#!/usr/bin/python3

import argparse
import sys

pre_parser = argparse.ArgumentParser(add_help=False)
pre_parser.add_argument('--format', choices=['avrdude', 'cfg'], default='cfg',
                    help="Format to write out fuses in (AVRDude CLI parameters or minipro config)")
pre_parser.add_argument('--lock', choices=['none', 'program', 'verify'], default='none',
                    help="Lock the device from further programming and/or verification.")
pre_parser.add_argument('--self-program', action='store_true',
                    help="Enable self-programming of the program flash.")
pre_parser.add_argument('--disable-reset', action='store_true',
                    help="Disable the external reset pin. WARNING: will require high voltage programming to modify chip.")
pre_parser.add_argument('--debugwire', action='store_true',
                    help="Enable DebugWIRE interface.")
pre_parser.add_argument('--disable-spi', action='store_true',
                    help="Disable SPI programming interface.")
pre_parser.add_argument("--always-watchdog", action="store_true",
                    help="Force watchdog timer to always be enabled.")
pre_parser.add_argument("--preserve-eeprom", action='store_true',
                    help="Preserve EEPROM through chip erase.")
pre_parser.add_argument("--bod-level", choices=[
                        "none",
                        "1.8V",
                        "2.7V",
                        "4.3V"
                    ], default="none",
                    help="Set threshold level for brownout detection.")
pre_parser.add_argument('--no-prescale', action='store_true',
                    help="Start in 1:1 clock prescale rather than 1:8.")
pre_parser.add_argument('--clk-out', action='store_true',
                    help="Enable buffering internal clock to GPIO.")
pre_parser.add_argument('--startup-time', choices=[
                        "none",
                        "4ms",
                        "64ms"
                    ], default="64ms",
                    help="Set startup delay for internal oscillator.")
pre_parser.add_argument('--clk-source', choices=[
                        "8MHz",
                        "6.4MHz",
                        "128kHz",
                        "pll",
                        "external"
                    ], default="8MHz",
                    help="Select clock source at boot. Note - 6.4MHz mode will put the chip into ATTiny15 Compatibility Mode which may cause other flags to work unreliably.")
pre_args, _ = pre_parser.parse_known_args()

parser = argparse.ArgumentParser(description="ATTiny85 fuse byte generator. Defaults to factory settings for any non-specified arguments. ATTiny15 mode not fully supported.")
# a bit of cheating (poking at internal representation) to ensure all the arguments end up in the help text if they should
# dear argparse pls give me a better way to do this
[parser._add_action(a) for a in pre_parser._actions]

if pre_args.clk_source == 'external' or pre_args.clk_source == 'pll':
    parser.add_argument('--startup-cycles', choices=[258, 1024, 16384], default=16384,
                        help="Number of startup cycles for crystal oscillator.")
if pre_args.clk_source == 'external':
    parser.add_argument('--clk-freq', type=float, default=8e6,
                        help="Frequency of external oscillator in Hz.")

args = parser.parse_args()

lock_byte = {
    "none": 0b11111111,
    "program": 0b11111110,
    "verify": 0b11111100
}[args.lock]

fuse_ext = 0b11111110 if args.self_program else 0b11111111

fuse_high = (
      (0b00000000 if args.disable_reset else 0b10000000)
    | (0b00000000 if args.debugwire else 0b01000000)
    | (0b00100000 if args.disable_spi else 0b00000000)
    | (0b00000000 if args.always_watchdog else 0b00010000)
    | (0b00000000 if args.preserve_eeprom else 0b00001000)
    | ({
        'none': 0b111,
        '1.8V': 0b110,
        '2.7V': 0b101,
        '4.3V': 0b100
    }[args.bod_level])
)

if args.clk_source == '6.4MHz':
    print("6.4MHz mode not yet supported")
    sys.exit(1)
elif args.clk_source == '8MHz':
    cksel = 0b0010
    sut = {
        'none': 0b00,
        '4ms': 0b01,
        '64ms': 0b10
    }[args.startup_time]
elif args.clk_source == '128kHz':
    cksel = 0b0100
    sut = {
        'none': 0b00,
        '4ms': 0b01,
        '64ms': 0b10
    }[args.startup_time]
elif args.clk_source == 'external':
    print("External mode not yet supported")
    sys.exit(2)

fuse_low = (
      (0b10000000 if args.no_prescale else 0b00000000)
    | (0b00000000 if args.clk_out else 0b01000000)
    | (sut << 4)
    | cksel
)

if args.format == 'avrdude':
    print(f"-U lfuse:w:0x{fuse_low:02x}:m -U hfuse:w:0x{fuse_high:02x}:m -U efuse:w:0x{fuse_ext:02x}:m -U lock:w:0x{lock_byte:02x}:m")
else:
    print(f"""fuses_lo = 0x{fuse_low:02x}
    fuses_hi = 0x{fuse_high:02x}
    fuses_ext = 0x{fuse_ext:02x}
    lock_byte = 0x{lock_byte:02x}""")
// Spec header format proposal--B. Graner 2020-05-15
// The overall format of a spec header would be 8192 bits.
// The header would be divided into 8 sections, each of
// which would be divided into 16 64-bit words. The
// meaning of each 64-bit word in the file would be
// interpreted according to the following list, which
// can be expanded according to the entries and/or sections
// marked 'TBD'. Consistent use of 64 bit words means the
// first bit of each entry x.y in the table can be indexed
// according to 1024*x+64*y

//Section 0: Source information and sanity checks
//	0.0: Expected file length (bytes):
//	0.1: UTC time stamp:
//	0.2: Input species:
//	0.3: Input flux:
//	0.4: TBD
//	0.5: TBD
//	0.6: TBD
//	0.7: TBD
//	0.8: TBD
//	0.9: TBD
//	0.10: TBD
//	0.11: TBD
//	0.12: TBD
//	0.13: TBD
//	0.14: TBD
//	0.15: TBD

//Section 1: ADC/FFT information
//	1.0: Max Freq (MHz):       0.0
//	1.1: Min Freq (MHz):       1600.00
//	1.2: Samples per spectrum: 8192
//	1.3: Raw spectra averaged: 8
//	1.4: Sample length (bits): 8
//	1.5: Packet header length (bits): 256
//	1.6: TBD
//	1.7: TBD
//	1.8: TBD
//	1.9: TBD
//	1.10: TBD
//	1.11: TBD
//	1.12: TBD
//	1.13: TBD
//	1.14: TBD
//	1.15: TBD

//Section 2: Gas pressure information:
//	2.0: Total Pressure (Torr):
//	2.1: Species 1 Mol. Mass:
//	2.2: Partial_Pressure (Torr):
//	2.3: Species 2 Mol. Mass:
//	2.4: Partial_Pressure (Torr):
//	2.5: Species 3 Mol. Mass:
//	2.6: Partial_Pressure (Torr):
//	2.7: Species 4 Mol. Mass:
//	2.8: Partial_Pressure (Torr):
//	2.9: Species 5 Mol. Mass:
//	2.10: Partial_Pressure (Torr):
//	2.11: Species 6 Mol. Mass:
//	2.12: Partial_Pressure (Torr):
//	2.13: Species 7 Mol. Mass:
//	2.14: Partial_Pressure (Torr):
//	2.15: TBD

//Section 3: Magnetic field informtation
//	3.0: NMR probe reading (T):
//	3.1: Main power supply current (A):
//	3.2: Trap coil current (mA):
//	3.3: X shim coil current (A):
//	3.4: Y shim coil current (A):
//	3.5: Z shim coil current (A):
//	3.6: XY shim coil current (A):
//	3.7: YZ shim coil current (A):
//	3.8: Z^2 shim coil current (A):
//	3.8: TBD
//	3.9: TBD
//	3.10: TBD
//	3.11: TBD
//	3.12: TBD
//	3.13: TBD
//	3.14: TBD
//	3.15: TBD


//Section 4: Temperature information
//	4.0: Decay cell input end temp (K):
//	4.1: Decay cell far end temp (K):
//	4.2: J-end amplifier temp (K):
//	4.3: Straight-end amplifier temp (K):
//	4.4: Coil form temp (K):
//	4.5:
//	4.6:
//	4.7:
//	4.8:
//	4.9:
//	4.10:
//	4.11:
//	4.12:
//	4.13:
//	4.14:
//	4.15:

//Section 5: TBD
//Section 6: TBD
//Section 7: TBD
//Section 8: TBD

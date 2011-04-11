	# how much to grow the pads by for soldermask
	# clearance from planes
# The symbol is backwards pin-wise
Element(0x00 "SMT diode (pin 2 is cathode)" "" "DO214AC" 0 0 227 0 3 100 0x00)
(
	ElementLine(-217 -92 -217 92 20)
	    ElementLine(-217 92 -145 118 10)
	    ElementLine(-145 118 207 118 10)
	    ElementLine(207 118 207 -118 10)
	    ElementLine(207 -118 -145 -118 10)
	    ElementLine(-145 -118 -217 -92 10)
	Pad(-109 -33 
		-109 33
		109 33 121 "2" "2" 0x00000100)
	    Pad(109 -33 
		109 33
		109 33 151 "1" "1" 0x00000100)
)

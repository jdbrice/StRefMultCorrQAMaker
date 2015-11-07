
# for command line args
import sys
# for regex
import re

findRN = re.compile( r"(st_physics)_(adc_)?([0-9]*)")

if ( len(sys.argv) > 1 ) :
	filename = sys.argv[ 1 ]
else :
	filename = "st_physics.lis"

if ( len(sys.argv) > 2 ) :
	filenameOut = sys.argv[ 2 ]
else :
	filenameOut = "RL.lis"

print "converting to Run List"
with open ( filename, "r") as myfile:
	data = myfile.read()
	f = open( filenameOut, 'w' )

	seen = set()
	result = []
	rei = findRN.finditer( data );
	for m in rei :
		#print m.start(), " - ", m.end(), " : ", m.group( 3 )
		item = m.group( 3 )
		iItem = int(item)
		if iItem not in seen :
			seen.add( iItem )
			result.append( iItem )

	result.sort()
	for i in result :
		f.write( str(i) + ", \n" )




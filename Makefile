

includes 	= $JDB_LIB/include/:#:#StRoot:#.sl64_gcc447/include:/afs/rhic.bnl.gov/star/ROOT/5.34.09/.sl64_gcc447/rootdeb/include:$JDB_LIB/include/jdb/
libs 		= "$(JDB_LIB)/lib/libJDB.so"

all:
	cons LIBS=$(libs) CPPPATH="$(JDB_LIB)/include/jdb/:$(JDB_LIB)/include/:#:#StRoot:#.sl64_gcc447/include:/afs/rhic.bnl.gov/star/ROOT/5.34.09/.sl64_gcc447/rootdeb/include"
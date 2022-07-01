all:
	g++ fitscli.cpp vipsoperations.cpp fits.cpp ConsoleTable.cpp `pkg-config vips-cpp --cflags --libs`  -lcfitsio -ltiff -o fitscli


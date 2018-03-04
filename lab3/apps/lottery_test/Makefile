default:
	cd userprog; make
	#cd dynamic_test; make

clean:
	cd userprog; make clean
	#cd dynamic_test; make clean

run:
	cd ../../bin; dlxsim -x os.dlx.obj -a -u userprog4.dlx.obj 2; ee469_fixterminal


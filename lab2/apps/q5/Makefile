default:
	cd makeprocs; make
	cd one; make
	cd two; make
	cd three; make
	cd four; make
	cd five; make

clean:
	cd makeprocs; make clean
	cd one; make clean
	cd two; make clean
	cd three; make clean
	cd four; make clean
	cd five; make clean

run:
	cd ../../bin; dlxsim -x os.dlx.obj -a -u makeprocs.dlx.obj 5 6; ee469_fixterminal

test:
	../avrsvf0 -datmega16 -s -e -iftest_m16.hex -pf -vf -F -ovtest_tc1.svf -mp -f0xFF89FF
	diff -d --strip-trailing-cr -I "//.*" test_tc1.svf test_tc1.ref
	../avrsvf0 -datmega128 -chd1 -ctd0 -chi8 -cti0 -s -e -iftest_m128.hex -pf -vf -F -ovtest_tc2.svf -mp -f0xFF89FF
	diff -d --strip-trailing-cr -I "//.*" test_tc2.svf test_tc2.ref

clean:
	rm *.svf


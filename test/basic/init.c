#include <twtest.h>

int main (int argc, char *argv[]) {
	terr_t err, nerr = 0;

	PRINT_TEST_MSG ("Check if TaskWork can initialize and finalize properly");

	err = TW_Init (&argc, &argv);
	CHECK_ERR
	err = TW_Finalize ();
	CHECK_ERR

	PRINT_TEST_RESULT
	return nerr;
}
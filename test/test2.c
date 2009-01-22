#include <stdlib.h>
#include <exception.h>

static
void func2(void)
{
	throw(1, "test error");
}

static
void func1(void)
{
	func2();
}

int main(int argc, char *argv[])
{
	int rc = 0;

	try {
		func1();
		rc = 1;
	} except {
		on (1) {
			exception_dump(STDERR_FILENO);
		} finally {
			exception_dump(STDERR_FILENO);
			rc = 1;
		}
	}

	return rc;
}

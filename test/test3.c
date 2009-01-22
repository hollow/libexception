#include <stdlib.h>
#include <exception.h>

static
void func2(void)
{
}

static
void func1(void)
{
	func2();
}

int main(int argc, char *argv[])
{
	int rc = 1;

	try {
		func1();
		func2();
		rc = 0;
	} except {
		finally {
			exception_dump(STDERR_FILENO);
		}
	}

	return rc;
}

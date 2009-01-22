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
	try { func2(); }
	except { continue; }
}

int main(int argc, char *argv[])
{
	int rc = 1;

	try {
		func1();
	} except {
		on (1) {
			exception_dump(STDERR_FILENO);
			rc = 0;
		} finally {
			exception_dump(STDERR_FILENO);
		}
	}

	return rc;
}

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
		try {
			func1();
		} except {
			continue;
		}
	} except {
		exception_dump(STDERR_FILENO);

		on (1) {
			rc = 0;
		} finally {
			continue;
		}
	}

	return rc;
}

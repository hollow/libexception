#include <stdlib.h>
#include <exception.h>

static
void func2(void)
{
	throw(1, "test error");
}

int main(int argc, char *argv[])
{
	int rc = 1;

	try {
		try {
			try {
				func2();
			} except {
				exception_dump(STDERR_FILENO);

				on (1) {
					rc--;
				} finally {
					continue;
				}
			}

			throw(2, "test error");
		} except {
			exception_dump(STDERR_FILENO);

			on (2) {
				rc--;
			} finally {
				continue;
			}
		}

		throw(3, "test error");
	} except {
		exception_dump(STDERR_FILENO);

		on (3) {
			rc--;
		} finally {
			continue;
		}
	}

	return rc;
}

#define DOCTEST_CONFIG_IMPLEMENTATION_IN_DLL
#include <doctest.h>

int main(int argc, char *argv[])
{
  doctest::Context context(argc, argv);
  return context.run();
}
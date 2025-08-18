set PROJ=%CD%
set TESTEXE=%PROJ%\build-tests\Debug\Connection_Machine_tests.exe

mkdir coverage

OpenCppCoverage ^
  --sources %PROJ%\src ^
  --cover_children ^
  --export_type cobertura:coverage\coverage.xml ^
  --export_type html:coverage ^
  -- "%TESTEXE%" --gtest_output=xml:coverage\gtest.xml
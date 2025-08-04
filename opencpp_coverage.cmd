set PROJ_SRC=%CD%\src
mkdir coverage
OpenCppCoverage ^
  --sources "%PROJ_SRC%\*" ^
  --cover_children ^
  --export_type cobertura:coverage\coverage.xml ^
  --export_type html:coverage ^
  -- build-tests\Debug\Connection_Machine_tests.exe ^
  --gtest_output=xml:coverage\gtest.xml